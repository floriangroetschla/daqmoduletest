//
// Created by fgrotsch on 3/4/21.
//

#include "Consumer.hpp"

namespace dunedaq {
    namespace daqmoduletest {
        Consumer::Consumer(const std::string& name) : dunedaq::appfwk::DAQModule(name), thread_(std::bind(&Consumer::do_work, this, std::placeholders::_1)), inputQueue() {
            register_command("start", &Consumer::do_start);
            register_command("stop", &Consumer::do_stop);
        }

        void Consumer::init(const nlohmann::json& init_data) {

        }

        void Consumer::get_info(opmonlib::InfoCollector& ci, int level) {

        }

        void Consumer::do_start(const nlohmann::json& args) {

        }

        void Consumer::do_stop(const nlohmann::json& args) {

        }

        void Consumer::do_work(std::atomic<bool>& running_flag) {

        }
    }
}