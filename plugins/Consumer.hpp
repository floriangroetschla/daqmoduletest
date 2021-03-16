//
// Created by fgrotsch on 3/4/21.
//

#ifndef DAQMODULETEST_CONSUMER_HPP
#define DAQMODULETEST_CONSUMER_HPP

#include "appfwk/DAQModule.hpp"
#include "appfwk/DAQSource.hpp"
#include "appfwk/ThreadHelper.hpp"
#include "daqmoduletest/conf/Structs.hpp"
#include <boost/align/aligned_allocator.hpp>

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
        void do_start_measurement(const nlohmann::json& obj);
        void do_stop_measurement(const nlohmann::json& obj);
        consumerinfo::Info collect_info();

        dunedaq::appfwk::ThreadHelper thread_;
        void do_work(std::atomic<bool>&);

        using source_t = dunedaq::appfwk::DAQSource<std::vector<int, boost::alignment::aligned_allocator<int, 512>>>;
        std::unique_ptr<source_t> inputQueue;

        std::atomic<uint64_t> m_bytes_received{0};
        std::atomic<uint64_t> m_bytes_written{0};
        std::atomic<uint64_t> m_measured_bytes_written{0};
        std::atomic<bool> m_do_measurement{false};
        //std::ofstream m_output_stream;
        int fd;
        std::string m_output_file;
        std::ofstream m_log_stream;
        std::chrono::steady_clock::time_point m_time_of_start_work;
        std::chrono::steady_clock::time_point m_time_of_completion;
        std::chrono::steady_clock::time_point m_time_of_start_measurement;
        std::chrono::steady_clock::time_point m_time_of_stop_measurement;
        std::atomic<bool> m_completed_work{false};
        std::atomic<bool> m_completed_measurement{false};
        conf::Conf m_conf;
        std::mutex m_start_lock;
    };
    }
}

#endif //DAQMODULETEST_CONSUMER_HPP
