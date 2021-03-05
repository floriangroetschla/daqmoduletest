//
// Created by fgrotsch on 3/4/21.
//

#include "RandomProducer.hpp"
#include "ers/Issue.hpp"
#include "appfwk/DAQModuleHelper.hpp"

#include <vector>

namespace dunedaq {
    namespace daqmoduletest {

        RandomProducer::RandomProducer(const std::string& name) : dunedaq::appfwk::DAQModule(name), thread_(std::bind(&RandomProducer::do_work, this, std::placeholders::_1)), mt_rand{} {
            register_command("start", &RandomProducer::do_start);
            register_command("stop", &RandomProducer::do_stop);
        }

        void RandomProducer::init(const nlohmann::json& init_data) {
            try {
                auto qi = appfwk::queue_index(init_data, {"q1"});
                outputQueue.reset(new sink_t(qi["q1"].inst));
            } catch (const ers::Issue& excpt) {
                std::cout << "Could not initialize queue" << std::endl;
            }
        }

        void RandomProducer::get_info(opmonlib::InfoCollector& /*ci*/, int /*level*/) {

        }

        void RandomProducer::do_start(const nlohmann::json& /*args*/) {
            thread_.start_working_thread();
        }

        void RandomProducer::do_stop(const nlohmann::json& /*args*/) {
            thread_.stop_working_thread();
        }

        void RandomProducer::do_work(std::atomic<bool>& running_flag) {
            while (running_flag.load()) {
                for (uint i = 0; i < NUMBER_OF_BUFFER_ELEMENTS; ++i) {
                    message_buffer.buffer[i] = mt_rand();
                }
                try {
                    outputQueue->push(message_buffer, std::chrono::milliseconds(100));
                } catch (const dunedaq::appfwk::QueueTimeoutExpired& excpt) {
                    std::cout << "Could not push to queue" << std::endl;
                }
                std::cout << "Pushed message" << std::endl;
                //std::cout << "Pushed random sequence " << message_buffer << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }
    }
}

DEFINE_DUNE_DAQ_MODULE(dunedaq::daqmoduletest::RandomProducer)