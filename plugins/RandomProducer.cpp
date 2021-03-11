//
// Created by fgrotsch on 3/4/21.
//

#include "daqmoduletest/randomproducerinfo/Nljs.hpp"
#include "daqmoduletest/conf/Nljs.hpp"
#include "RandomProducer.hpp"
#include "ers/Issue.hpp"
#include "appfwk/DAQModuleHelper.hpp"
#include "logging/Logging.hpp"
#include "Issues.h"

#include <vector>

namespace dunedaq {
    namespace daqmoduletest {

        RandomProducer::RandomProducer(const std::string& name) : dunedaq::appfwk::DAQModule(name), thread_(std::bind(&RandomProducer::do_work, this, std::placeholders::_1)), mt_rand{}, m_start_lock{} {
            register_command("start", &RandomProducer::do_start);
            register_command("stop", &RandomProducer::do_stop);
        }

        void RandomProducer::init(const nlohmann::json& init_data) {
            try {
                auto qi = appfwk::queue_index(init_data, {"outputQueue"});
                outputQueue.reset(new sink_t(qi["outputQueue"].inst));
            } catch (const ers::Issue& excpt) {
                throw QueueFatalError(ERS_HERE, get_name(), "outputQueue", excpt);
            }
            m_start_lock.lock();
            thread_.start_working_thread(get_name());
        }

        void RandomProducer::get_info(opmonlib::InfoCollector& ci, int /*level*/) {
            randomproducerinfo::Info producerInfo;

            producerInfo.bytes_sent = m_bytes_sent.load();
            ci.add(producerInfo);
        }

        void RandomProducer::do_start(const nlohmann::json& args) {
            m_conf = args.get<conf::Conf>();
            m_start_lock.unlock();
            TLOG() << get_name() << " successfully started";
        }

        void RandomProducer::do_stop(const nlohmann::json& /*args*/) {
            thread_.stop_working_thread();
            TLOG() << get_name() << " successfully stopped";
        }

        void RandomProducer::do_work(std::atomic<bool>& running_flag) {
            std::lock_guard<std::mutex> lock_guard(m_start_lock);
            while (running_flag.load()) {
                std::vector<int> buffer(m_conf.message_size / sizeof(int));
                for (uint i = 0; i < buffer.size(); ++i) {
                    buffer[i] = mt_rand();
                }
                try {
                    outputQueue->push(buffer, std::chrono::milliseconds(100));
                    m_bytes_sent += m_conf.message_size;
                } catch (const dunedaq::appfwk::QueueTimeoutExpired& excpt) {
                    std::ostringstream oss_warn;
                    oss_warn << "push to output queue \"" << outputQueue->get_name() << "\"";
                    ers::warning(dunedaq::appfwk::QueueTimeoutExpired(ERS_HERE, get_name(), oss_warn.str(),1000));
                }
            }
        }
    }
}

DEFINE_DUNE_DAQ_MODULE(dunedaq::daqmoduletest::RandomProducer)