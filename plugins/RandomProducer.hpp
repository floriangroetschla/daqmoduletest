//
// Created by fgrotsch on 3/4/21.
//

#ifndef DAQMODULETEST_RANDOMPRODUCER_HPP
#define DAQMODULETEST_RANDOMPRODUCER_HPP

#include "appfwk/DAQModule.hpp"
#include "appfwk/DAQSink.hpp"
#include "appfwk/ThreadHelper.hpp"
#include "appfwk/app/Nljs.hpp"

#include <atomic>
#include <random>

namespace dunedaq {
    namespace daqmoduletest {

    class RandomProducer : public dunedaq::appfwk::DAQModule {
    public:
        explicit RandomProducer(const std::string& name);

        RandomProducer(const RandomProducer&) = delete;
        RandomProducer& operator=(const RandomProducer&) = delete;
        RandomProducer& operator=(RandomProducer&&) = delete;
        RandomProducer(RandomProducer&&) = delete;

        void init(const nlohmann::json& obj) override;
        void get_info(opmonlib::InfoCollector& ci, int level) override;

    private:
        void do_start(const nlohmann::json& obj);
        void do_stop(const nlohmann::json& obj);

        dunedaq::appfwk::ThreadHelper thread_;
        void do_work(std::atomic<bool>&);

        using sink_t = dunedaq::appfwk::DAQSink<int>;
        std::unique_ptr<sink_t> outputQueue;

        std::mt19937 mt_rand;
    };
    }
}

#endif //DAQMODULETEST_RANDOMPRODUCER_HPP