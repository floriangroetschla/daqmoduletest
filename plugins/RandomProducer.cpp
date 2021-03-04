//
// Created by fgrotsch on 3/4/21.
//

#include "RandomProducer.hpp"

#include <vector>

namespace dunedaq {
    namespace daqmoduletest {

        RandomProducer::RandomProducer(const std::string& name) : dunedaq::appfwk::DAQModule(name), thread_(std::bind(&RandomProducer::do_work, this, std::placeholders::_1)), outputQueue() {
            register_command("start", &RandomProducer::do_start);
            register_command("stop", &RandomProducer::do_stop);
        }

        void RandomProducer::init(const nlohmann::json& init_data) {
            
        }

        void RandomProducer::get_info(opmonlib::InfoCollector& ci, int level) {

        }

        void RandomProducer::do_start(const nlohmann::json& args) {

        }

        void RandomProducer::do_stop(const nlohmann::json& args) {

        }

        void RandomProducer::do_work(std::atomic<bool>& running_flag) {

        }
    }
}