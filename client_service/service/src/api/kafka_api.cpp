#include "api/kafka_api.h"

#include "shared/errors/error.h"
#include "api/squadov_api.h"

namespace service::api {
namespace {

constexpr int MAX_KAFKA_RETRY = 10;

}

void KafkaApi::initialize() {
    const auto info = getGlobalApi()->getKafkaInfo();

    std::unique_ptr<RdKafka::Conf> conf(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));

    std::string kafkaErr;
    if (conf->set("bootstrap.servers", info.servers, kafkaErr) != RdKafka::Conf::CONF_OK) {
        THROW_ERROR("Failed to set bootstrap.servers [Kafka]: " << kafkaErr);
        return;
    }

    if (conf->set("client.id", getGlobalApi()->getSessionUserUuid(), kafkaErr) != RdKafka::Conf::CONF_OK) {
        THROW_ERROR("Failed to set client.id [Kafka]: " << kafkaErr);
        return;
    }

    if (conf->set("security.protocol", "SASL_SSL", kafkaErr) != RdKafka::Conf::CONF_OK) {
        THROW_ERROR("Failed to set security.protocol [Kafka]: " << kafkaErr);
        return;
    }

    if (conf->set("sasl.mechanisms", "PLAIN", kafkaErr) != RdKafka::Conf::CONF_OK) {
        THROW_ERROR("Failed to set sasl.mechanisms [Kafka]: " << kafkaErr);
        return;
    }

    if (conf->set("sasl.username", info.key, kafkaErr) != RdKafka::Conf::CONF_OK) {
        THROW_ERROR("Failed to set sasl.username [Kafka]: " << kafkaErr);
        return;
    }

    if (conf->set("sasl.password", info.secret, kafkaErr) != RdKafka::Conf::CONF_OK) {
        THROW_ERROR("Failed to set sasl.password [Kafka]: " << kafkaErr);
        return;
    }

    _producer = std::unique_ptr<RdKafka::Producer>(RdKafka::Producer::create(conf.get(), kafkaErr));
    if (!_producer) {
        THROW_ERROR("Failed to create Kafka producer: " << kafkaErr);
        return;
    }

    _wowTopic = info.wowTopic;

    // This thread is needed to call poll() at frequent intervals so that the
    // application callbacks get called.
    _producerThread = std::thread([this](){
        while (_running) {
            _producer->poll(0);
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        _producer->flush(10 * 1000);
    });
}

KafkaApi::~KafkaApi() {
    _running = false;
    _producerThread.join();
}

void KafkaApi::uploadWoWCombatLogLine(const std::string& matchViewUuid, const game_event_watcher::RawWoWCombatLog& log) const {
    const auto logJson = log.toJson().dump();
    genericKafkaProduce(log.timestamp, _wowTopic, logJson, &matchViewUuid);
}

void KafkaApi::genericKafkaProduce(const shared::TimePoint& tm, const std::string& topic, const std::string& message, const std::string* key) const {
    bool success = false;
    for (auto i = 0; i < MAX_KAFKA_RETRY; ++i) {
        const auto err = _producer->produce(
            topic,                                                          // Topic
            RdKafka::Topic::PARTITION_UA,                                   // Partition
            RdKafka::Producer::RK_MSG_COPY,                                 // Copy payload
            (void*)message.c_str(), message.size(),                         // Value
            key ? (void*)key->c_str() : nullptr, key ? key->size() : 0,     // Key
            shared::timeToUnixMs(tm),                                       // Timestamp
            nullptr                                                         // Message Opaque
        );

        if (err != RdKafka::ERR_NO_ERROR) {
            if (err == RdKafka::ERR__QUEUE_FULL) {
                LOG_INFO("Retrying Kafka produce: " << topic << "\t" << i << std::endl);
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
                continue;
            }

            THROW_ERROR("Failed to Kafka produce: " << topic << "\t -- " << RdKafka::err2str(err));
            return;
        }

        success = true;
        break;
    }

    if (!success) {
        THROW_ERROR("Queue was full for too long in Kafka produce: " << topic);
    }
}

KafkaApi* getKafkaApi() {
    static KafkaApiPtr global = std::make_unique<KafkaApi>();
    return global.get();
}

}