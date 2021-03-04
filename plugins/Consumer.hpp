//
// Created by fgrotsch on 3/4/21.
//

#ifndef DAQMODULETEST_CONSUMER_HPP
#define DAQMODULETEST_CONSUMER_HPP

#include "appfwk/DAQModule.hpp"
#include "appfwk/DAQSource.hpp"
#include "appfwk/ThreadHelper.hpp"

#include <atomic>

namespace dunedaq {
    namespace daqmoduletest {

    class Consumer : public dunedaq::appfwk::DAQModule {
    public:
        explicit Consumer(const std::string& name);

        Consumer(const Consumer&) = delete;
        Consumer& operator=(const Consumer&) = delete;
        Consumer(Consumer&&) = delete;
        Consumer& operator=(Consumer&&) = delete;

        void init(const nlohmann::json& obj) override;
        void get_info(opmonlib::InfoCollector& ci, int level) override;

    private:
        void do_start(const nlohmann::json& obj);
        void do_stop(const nlohmann::json& obj);

        dunedaq::appfwk::ThreadHelper thread_;
        void do_work(std::atomic<bool>&);

        using source_t = dunedaq::appfwk::DAQSource<int>;
        std::unique_ptr<source_t> inputQueue;
    };
    }
}

#endif //DAQMODULETEST_CONSUMER_HPP
