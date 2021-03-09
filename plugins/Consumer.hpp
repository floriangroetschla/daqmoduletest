//
// Created by fgrotsch on 3/4/21.
//

#ifndef DAQMODULETEST_CONSUMER_HPP
#define DAQMODULETEST_CONSUMER_HPP

#include "appfwk/DAQModule.hpp"
#include "appfwk/DAQSource.hpp"
#include "appfwk/ThreadHelper.hpp"
#include "Message.hpp"
#include <iostream>
#include <fstream>

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

        using source_t = dunedaq::appfwk::DAQSource<Message>;
        std::unique_ptr<source_t> inputQueue;

        Message message_buffer;
        std::atomic<uint64_t> m_bytes_received{0};
        std::atomic<uint64_t> m_bytes_written{0};
        std::ofstream m_output_stream;
        std::string m_output_file;
        std::ofstream m_log_stream;
        std::chrono::steady_clock::time_point m_time_of_start_work;
        std::chrono::steady_clock::time_point m_time_of_completion;
        std::atomic<bool> m_completed_work{false};
    };
    }
}

#endif //DAQMODULETEST_CONSUMER_HPP
