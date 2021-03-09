//
// Created by fgrotsch on 3/4/21.
//

#include "daqmoduletest/consumerinfo/Nljs.hpp"
#include "daqmoduletest/conf/Nljs.hpp"
#include "Consumer.hpp"

#include "appfwk/DAQModuleHelper.hpp"
#include "logging/Logging.hpp"

namespace dunedaq {
    namespace daqmoduletest {
        Consumer::Consumer(const std::string& name) : dunedaq::appfwk::DAQModule(name), thread_(std::bind(&Consumer::do_work, this, std::placeholders::_1)), inputQueue() {
            register_command("start", &Consumer::do_start);
            register_command("stop", &Consumer::do_stop);
        }

        void Consumer::init(const nlohmann::json& init_data) {
            std::string log_file = "runs/" + get_name() + "_log.jsonl";
            m_log_stream.open(log_file);
            try {
                auto qi = appfwk::queue_index(init_data, {"inputQueue"});
                inputQueue.reset(new source_t(qi["inputQueue"].inst));
            } catch (const ers::Issue& excpt) {
                TLOG() << "Could not initialize queue" << std::endl;
            }
        }

        void Consumer::get_info(opmonlib::InfoCollector& ci, int /*level*/) {
            consumerinfo::Info consumerInfo;

            consumerInfo.bytes_received = m_bytes_received.load();
            consumerInfo.bytes_written = m_bytes_written.load();
            consumerInfo.completed = m_completed_work.load();
            consumerInfo.message_size = m_conf.message_size;
            consumerInfo.timestamp = std::time(0);
            consumerInfo.throughput = 0.0;
            if (m_completed_work) {
                std::chrono::duration<double> time_passed = std::chrono::duration_cast<std::chrono::duration<double>>(m_time_of_completion - m_time_of_start_work);
                consumerInfo.throughput = static_cast<double>(m_bytes_received) / 1000000.0 / time_passed.count();
            }
            ci.add(consumerInfo);

            nlohmann::json j;
            consumerinfo::to_json(j, consumerInfo);
            m_log_stream << j.dump() << std::endl;
        }

        void Consumer::do_start(const nlohmann::json& args) {
            m_conf = args.get<conf::Conf>();
            if (remove(m_conf.output_file.c_str()) == 0) {
                TLOG() << "Removed existing output file from previous run" << std::endl;
            }
            thread_.start_working_thread();
            TLOG() << get_name() << " successfully started";
        }

        void Consumer::do_stop(const nlohmann::json& /*args*/) {
            thread_.stop_working_thread();
            TLOG() << get_name() << " successfully stopped";
        }

        void Consumer::do_work(std::atomic<bool>& running_flag) {
            m_output_stream.open(m_conf.output_file);
            m_time_of_start_work = std::chrono::steady_clock::now();
            while (running_flag.load() && m_bytes_written < m_conf.bytes_to_send) {
                std::vector<int> buffer(m_conf.message_size / sizeof(int));
                try {
                    inputQueue->pop(buffer, std::chrono::milliseconds(10000));
                    m_bytes_received += m_conf.message_size;
                    m_output_stream.write((char*)buffer.data(), m_conf.message_size);
                    m_output_stream.flush();
                    m_bytes_written += m_conf.message_size;
                } catch (const dunedaq::appfwk::QueueTimeoutExpired& excpt) {
                    continue;
                }
            }
            m_time_of_completion = std::chrono::steady_clock::now();
            m_output_stream.close();
            m_completed_work = true;
        }
    }
}

DEFINE_DUNE_DAQ_MODULE(dunedaq::daqmoduletest::Consumer)