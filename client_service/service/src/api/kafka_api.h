#pragma once
#include <memory>
#include <string>
#include <librdkafka/rdkafkacpp.h>
#include <thread>

#include "game_event_watcher/wow/wow_log_watcher.h"

namespace service::api {

struct KafkaInfo {
    std::string servers;
    std::string key;
    std::string secret;
    std::string wowTopic;
};

class KafkaApi {
public:
    void initialize();
    void flush();
    ~KafkaApi();

    // WoW
    void uploadWoWCombatLogLine(const std::string& matchViewUuid, const game_event_watcher::RawWoWCombatLog& log) const;
private:
    void genericKafkaProduce(const shared::TimePoint& tm, const std::string& topic, const std::string& message, const std::string* key = nullptr) const;
    std::unique_ptr<RdKafka::Producer> _producer;
    std::thread _producerThread;
    bool _running = true;

    // Topics
    std::string _wowTopic;
};
using KafkaApiPtr = std::unique_ptr<KafkaApi>;

}