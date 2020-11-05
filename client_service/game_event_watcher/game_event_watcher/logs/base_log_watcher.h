#pragma once

#include "game_event_watcher/logs/log_watcher.h"
#include "shared/time.h"

#include <chrono>
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace game_event_watcher {

class BaseLogWatcher {
public:
    explicit BaseLogWatcher(bool useTimeChecks):
        _useTimeChecks(useTimeChecks) {
    }
    virtual ~BaseLogWatcher() = default;

    using EventCallback = std::function<void(const shared::TimePoint&, const void*)>;
    void notifyOnEvent(int event, const EventCallback& cb) {
        _callbacks[event].push_back(cb);
    }

    bool useTimeChecks() const { return _useTimeChecks; }
protected:
    void notify(int event, const shared::TimePoint& eventTime, const void* data, bool checkTime = true) const {
        if (checkTime && _useTimeChecks) {
            // Don't notify if the event time has drifted too far (probably due to reading a log that existed already).
            const auto maxDiff = std::chrono::seconds(10);
            const auto diff = shared::nowUtc() - eventTime;
            if (diff > maxDiff) {
                return;
            }
        }

        for (const auto& cb : _callbacks.at(event)) {
            cb(eventTime, data);
        }
    }

private:
    std::unordered_map<int, std::vector<EventCallback>> _callbacks;
    bool _useTimeChecks = false;
};

}