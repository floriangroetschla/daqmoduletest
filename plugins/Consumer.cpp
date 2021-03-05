//
// Created by fgrotsch on 3/4/21.
//

#include "Consumer.hpp"

#include "appfwk/DAQModuleHelper.hpp"

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
                std::cout << "Could not initialize queue" << std::endl;
            }
        }

        void Consumer::get_info(opmonlib::InfoCollector& /*ci*/, int /*level*/) {

        }

        void Consumer::do_start(const nlohmann::json& /*args*/) {
            thread_.start_working_thread();
        }

        void Consumer::do_stop(const nlohmann::json& /*args*/) {
            thread_.stop_working_thread();
        }

        void Consumer::do_work(std::atomic<bool>& running_flag) {
            while (running_flag.load()) {
                int r;
                try {
                    inputQueue->pop(r, std::chrono::milliseconds(100));
                    std::cout << "Received random number " << r << std::endl;
                } catch (const dunedaq::appfwk::QueueTimeoutExpired& excpt) {
                    continue;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }
    }
}

DEFINE_DUNE_DAQ_MODULE(dunedaq::daqmoduletest::Consumer)