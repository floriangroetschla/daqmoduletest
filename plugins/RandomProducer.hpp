//
// Created by fgrotsch on 3/4/21.
//

#ifndef DAQMODULETEST_RANDOMPRODUCER_HPP
#define DAQMODULETEST_RANDOMPRODUCER_HPP

#include "appfwk/DAQModule.hpp"
#include "appfwk/DAQSink.hpp"
#include "appfwk/ThreadHelper.hpp"
#include "appfwk/app/Nljs.hpp"
#include "Message.hpp"

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

        using sink_t = dunedaq::appfwk::DAQSink<Message>;
        std::unique_ptr<sink_t> outputQueue;

        std::mt19937 mt_rand;

        Message message_buffer;
        std::atomic<uint64_t> m_bytes_sent{0};
        uint64_t bytes_to_send{0};
    };

    }
}

#endif //DAQMODULETEST_RANDOMPRODUCER_HPP