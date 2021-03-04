//
// Created by fgrotsch on 3/4/21.
//

#include "RandomProducer.hpp"
#include "ers/Issue.hpp"

#include <vector>

namespace dunedaq {
    namespace daqmoduletest {

        RandomProducer::RandomProducer(const std::string& name) : dunedaq::appfwk::DAQModule(name), thread_(std::bind(&RandomProducer::do_work, this, std::placeholders::_1)), mt_rand{} {
            register_command("start", &RandomProducer::do_start);
            register_command("stop", &RandomProducer::do_stop);
        }

        void RandomProducer::init(const nlohmann::json& init_data) {
            auto ini = init_data.get<appfwk::app::ModInit>();
            for (const auto& qi : ini.qinfos) {
                if (qi.dir == "output") {
                    try {
                        outputQueue = std::unique_ptr<sink_t>(new sink_t(qi.inst));
                    } catch (const ers::Issue& excpt) {
                        std::cout << "Could not initialize queue" << std::endl;
                    }
                }
            }
        }

        void RandomProducer::get_info(opmonlib::InfoCollector& ci, int level) {

        }

        void RandomProducer::do_start(const nlohmann::json& args) {
            thread_.start_working_thread();
        }

        void RandomProducer::do_stop(const nlohmann::json& args) {
            thread_.stop_working_thread();
        }

        void RandomProducer::do_work(std::atomic<bool>& running_flag) {
            while (running_flag.load()) {
                int r = mt_rand();
                try {
                    outputQueue->push(r, std::chrono::milliseconds(100));
                } catch (const dunedaq::appfwk::QueueTimeoutExpired& excpt) {
                    std::cout << "Could not push to queue" << std::endl;
                }
                std::cout << "Pushed random number " << r << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }
    }
}