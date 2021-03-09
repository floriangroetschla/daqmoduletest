//
// Created by fgrotsch on 3/4/21.
//

#include "daqmoduletest/randomproducerinfo/Nljs.hpp"
#include "RandomProducer.hpp"
#include "ers/Issue.hpp"
#include "appfwk/DAQModuleHelper.hpp"
#include "logging/Logging.hpp"

#include <vector>

namespace dunedaq {
    namespace daqmoduletest {

        RandomProducer::RandomProducer(const std::string& name) : dunedaq::appfwk::DAQModule(name), thread_(std::bind(&RandomProducer::do_work, this, std::placeholders::_1)), mt_rand{} {
            register_command("start", &RandomProducer::do_start);
            register_command("stop", &RandomProducer::do_stop);
        }

        void RandomProducer::init(const nlohmann::json& init_data) {
            try {
                auto qi = appfwk::queue_index(init_data, {"outputQueue"});
                outputQueue.reset(new sink_t(qi["outputQueue"].inst));
            } catch (const ers::Issue& excpt) {
                TLOG() << "Could not initialize queue" << std::endl;
            }
        }

        void RandomProducer::get_info(opmonlib::InfoCollector& ci, int /*level*/) {
            randomproducerinfo::Info producerInfo;

            producerInfo.bytes_sent = m_bytes_sent.load();
            ci.add(producerInfo);
        }

        void RandomProducer::do_start(const nlohmann::json& /*args*/) {
            thread_.start_working_thread();
            TLOG() << get_name() << " successfully started";
        }

        void RandomProducer::do_stop(const nlohmann::json& /*args*/) {
            thread_.stop_working_thread();
            TLOG() << get_name() << " successfully stopped";
        }

        void RandomProducer::do_work(std::atomic<bool>& running_flag) {
            while ((m_bytes_sent < BYTES_TO_SEND) && running_flag.load()) {
                for (uint i = 0; i < NUMBER_OF_BUFFER_ELEMENTS; ++i) {
                    message_buffer.buffer[i] = mt_rand();
                }
                try {
                    outputQueue->push(message_buffer, std::chrono::milliseconds(100));
                    m_bytes_sent += MESSAGE_SIZE;
                } catch (const dunedaq::appfwk::QueueTimeoutExpired& excpt) {
                    TLOG() << "Could not push to queue" << std::endl;
                }
            }
        }
    }
}

DEFINE_DUNE_DAQ_MODULE(dunedaq::daqmoduletest::RandomProducer)