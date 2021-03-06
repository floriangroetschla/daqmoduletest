//
// Created by fgrotsch on 3/4/21.
//

#include "daqmoduletest/consumerinfo/Nljs.hpp"
#include "daqmoduletest/conf/Nljs.hpp"
#include "Consumer.hpp"
#include "Issues.h"
#include <boost/align/aligned_allocator.hpp>

#include "appfwk/DAQModuleHelper.hpp"
#include "logging/Logging.hpp"

namespace dunedaq {
    namespace daqmoduletest {
        Consumer::Consumer(const std::string& name) : dunedaq::appfwk::DAQModule(name), thread_(std::bind(&Consumer::do_work, this, std::placeholders::_1)), inputQueue(), m_start_lock{} {
            register_command("start", &Consumer::do_start);
            register_command("stop", &Consumer::do_stop);
            register_command("start_measurement", &Consumer::do_start_measurement);
            register_command("stop_measurement", &Consumer::do_stop_measurement);
        }

        void Consumer::init(const nlohmann::json& init_data) {
            std::string log_file = "runs/" + get_name() + "_log.jsonl";
            m_log_stream.open(log_file);
            if (!m_log_stream.is_open()) {
                throw FileError(ERS_HERE, get_name(), log_file);
            }
            try {
                auto qi = appfwk::queue_index(init_data, {"inputQueue"});
                inputQueue.reset(new source_t(qi["inputQueue"].inst));
            } catch (const ers::Issue& excpt) {
                throw QueueFatalError(ERS_HERE, get_name(), "inputQueue", excpt);
            }
            m_start_lock.lock();
            thread_.start_working_thread(get_name());
        }

        consumerinfo::Info Consumer::collect_info() {
            consumerinfo::Info consumerInfo;

            consumerInfo.bytes_received = m_bytes_received.load();
            consumerInfo.bytes_written = m_bytes_written.load();
            consumerInfo.completed = m_completed_work.load();
            consumerInfo.completed_measurement = m_completed_measurement.load();
            consumerInfo.message_size = m_conf.message_size;
            consumerInfo.timestamp = std::time(0);
            std::chrono::duration<double> time_passed = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - m_time_of_last_info);
            consumerInfo.throughput = static_cast<double>(m_bytes_since_last_info) / (1 << 20) / time_passed.count();
            if (m_completed_measurement) {
                std::chrono::duration<double> time_passed = std::chrono::duration_cast<std::chrono::duration<double>>(m_time_of_stop_measurement - m_time_of_start_measurement);
                consumerInfo.throughput_from_last_measurement = static_cast<double>(m_measured_bytes_written) / (1 << 20) / time_passed.count();
            } else {
                consumerInfo.throughput_from_last_measurement = 0;
            }
            return consumerInfo;
        }

        void Consumer::get_info(opmonlib::InfoCollector& ci, int /*level*/) {
            consumerinfo::Info consumerInfo = collect_info();
            ci.add(consumerInfo);
            m_time_of_last_info = std::chrono::steady_clock::now();
            m_bytes_since_last_info = 0;
        }

        void Consumer::do_start(const nlohmann::json& args) {
            m_conf = args.get<conf::Conf>();
            m_start_lock.unlock();
            TLOG() << get_name() << " successfully started";
        }

        void Consumer::do_stop(const nlohmann::json& /*args*/) {
            TLOG() << "Stopping " << get_name() << std::endl;
            thread_.stop_working_thread();
            TLOG() << get_name() << " successfully stopped";
        }

        void Consumer::do_start_measurement(const nlohmann::json& /*args*/) {
            TLOG() << "Starting measurement at " << get_name() << std::endl;
            m_do_measurement = true;
        }

        void Consumer::do_stop_measurement(const nlohmann::json& /*args*/) {
            TLOG() << "Stopping measurement at " << get_name() << std::endl;
            m_do_measurement = false;
        }

        void Consumer::do_work(std::atomic<bool>& running_flag) {
            std::lock_guard<std::mutex> lock_guard(m_start_lock);
            std::string output_file = m_conf.output_dir + "/output_" + get_name();
            if (remove(output_file.c_str()) == 0) {
                TLOG() << "Removed existing output file from previous run" << std::endl;
            }
            fd = open(output_file.c_str(), O_CREAT | O_RDWR | O_DIRECT);
            if (fd == -1) {
                throw FileError(ERS_HERE, get_name(), output_file);
            }

            std::vector<int, boost::alignment::aligned_allocator<int, 4096>> buffer(m_conf.message_size / sizeof(int));

            m_time_of_start_work = std::chrono::steady_clock::now();
            m_time_of_last_info = std::chrono::steady_clock::now();
            bool started_measuring = false;
            while (running_flag.load()) {
                try {
                    if (!started_measuring && m_do_measurement.load()) {
                        m_time_of_start_measurement = std::chrono::steady_clock::now();
                        started_measuring = true;
                    }
                    inputQueue->pop(buffer, std::chrono::milliseconds(100));
                    m_bytes_received += m_conf.message_size;
                    int write_result = write(fd, (char*)buffer.data(), m_conf.message_size);
                    if (write_result == -1) {
                        TLOG() << "Could not write to disk" << std::endl;
                    } else {
                        m_bytes_written += m_conf.message_size;
                        m_bytes_since_last_info += m_conf.message_size;
                        if (started_measuring) {
                            m_measured_bytes_written += m_conf.message_size;
                            if (!m_do_measurement.load()) {
                                // A measurement finished
                                m_time_of_stop_measurement = std::chrono::steady_clock::now();
                                started_measuring = false;
                                m_completed_measurement = true;

                                // Write result to file
                                consumerinfo::Info consumerInfo = collect_info();

                                nlohmann::json j;
                                consumerinfo::to_json(j, consumerInfo);
                                m_log_stream << j.dump() << std::endl;
                            }
                        }
                    }
                } catch (const dunedaq::appfwk::QueueTimeoutExpired& excpt) {
                    continue;
                }
            }

            if (started_measuring && !m_do_measurement) {
                m_time_of_stop_measurement = std::chrono::steady_clock::now();
                m_completed_measurement = true;

                consumerinfo::Info consumerInfo = collect_info();
                nlohmann::json j;
                consumerinfo::to_json(j, consumerInfo);
                m_log_stream << j.dump() << std::endl;
            }

            m_time_of_completion = std::chrono::steady_clock::now();
            //m_output_stream.close();
            close(fd);
            m_completed_work = true;
            //remove(output_file.c_str());

            TLOG() << get_name() << " finished writing" << std::endl;
        }
    }
}

DEFINE_DUNE_DAQ_MODULE(dunedaq::daqmoduletest::Consumer)