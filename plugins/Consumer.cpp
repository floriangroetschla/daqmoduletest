//
// Created by fgrotsch on 3/4/21.
//

#include "daqmoduletest/consumerinfo/Nljs.hpp"
#include "Consumer.hpp"

#include "appfwk/DAQModuleHelper.hpp"
#include "logging/Logging.hpp"

#define OUTPUT_FILE "output_file"

namespace dunedaq {
    namespace daqmoduletest {
        Consumer::Consumer(const std::string& name) : dunedaq::appfwk::DAQModule(name), thread_(std::bind(&Consumer::do_work, this, std::placeholders::_1)), inputQueue() {
            register_command("start", &Consumer::do_start);
            register_command("stop", &Consumer::do_stop);
        }

        void Consumer::init(const nlohmann::json& init_data) {
            try {
                auto qi = appfwk::queue_index(init_data, {"q1"});
                inputQueue.reset(new source_t(qi["q1"].inst));
            } catch (const ers::Issue& excpt) {
                TLOG() << "Could not initialize queue" << std::endl;
            }
            if (remove(OUTPUT_FILE) == 0) {
                TLOG() << "Removed existing output file from previous run" << std::endl;
            }
        }

        void Consumer::get_info(opmonlib::InfoCollector& ci, int /*level*/) {
            consumerinfo::Info consumerInfo;

            consumerInfo.bytes_received = m_bytes_received.load();
            consumerInfo.bytes_written = m_bytes_written.load();
            ci.add(consumerInfo);
        }

        void Consumer::do_start(const nlohmann::json& /*args*/) {
            thread_.start_working_thread();
            TLOG() << get_name() << " successfully started";
        }

        void Consumer::do_stop(const nlohmann::json& /*args*/) {
            thread_.stop_working_thread();
            TLOG() << get_name() << " successfully stopped";
        }

        void Consumer::do_work(std::atomic<bool>& running_flag) {
            m_output_stream.open(OUTPUT_FILE);
            while (running_flag.load()) {
                try {
                    inputQueue->pop(message_buffer, std::chrono::milliseconds(100));
                    m_bytes_received += MESSAGE_SIZE;
                    m_output_stream.write((char*)&message_buffer.buffer[0], MESSAGE_SIZE);
                    m_bytes_written += MESSAGE_SIZE;
                } catch (const dunedaq::appfwk::QueueTimeoutExpired& excpt) {
                    continue;
                }
            }
            m_output_stream.close();
        }
    }
}

DEFINE_DUNE_DAQ_MODULE(dunedaq::daqmoduletest::Consumer)