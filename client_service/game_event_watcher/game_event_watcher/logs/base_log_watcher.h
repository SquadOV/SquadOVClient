#pragma once

#include "game_event_watcher/logs/log_watcher.h"
#include "shared/time.h"
#include "shared/log/log.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace game_event_watcher {

class BaseLogWatcher {
public:
    BaseLogWatcher(bool useTimeChecks, const shared::TimePoint& timeThreshold):
        _useTimeChecks(useTimeChecks),
        _timeThreshold(timeThreshold) {
    }
    virtual ~BaseLogWatcher() = default;

    using EventCallback = std::function<void(const shared::TimePoint&, const void*)>;
    void notifyOnEvent(int event, const EventCallback& cb) {
        _callbacks[event].push_back(cb);
    }

    bool useTimeChecks() const { return _useTimeChecks; }
    void setUseTimeChecks(bool v) { _useTimeChecks = v; }
    bool isTimeBeforeThreshold(const shared::TimePoint& tp) const { return _useTimeChecks && tp < _timeThreshold; }
    const shared::TimePoint& timeThreshold() const { return _timeThreshold; }

protected:
    void notify(int event, const shared::TimePoint& eventTime, const void* data, bool checkTime = true, bool quiet = false) const {
        if (!quiet) {
            LOG_INFO("Notify Log Event [" << event << "] at " << shared::timeToStr(eventTime) << " - Check Time " << (checkTime && _useTimeChecks) << std::endl);
        }
        
        // Discard events that came before the time threshold. The time threshold is generally when we
        // detect that the game has opened - this prevents us from reading old log events.
        if (_useTimeChecks) {
            if (checkTime && isTimeBeforeThreshold(eventTime)) {
                LOG_WARNING("\tDiscarding event due to time threshold: " << shared::timeToStr(_timeThreshold) << std::endl);
                return;
            }

            if (checkTime && shared::isTimeInFuture(eventTime)) {
                LOG_WARNING("\tDiscarding event due to future time: " << shared::timeToStr(eventTime) << std::endl);
                return;
            }
        }

        const auto it = _callbacks.find(event);
        if (it == _callbacks.end()) {
            return;
        }

        for (const auto& cb : it->second) {
            cb(eventTime, data);
        }
    }

private:
    std::unordered_map<int, std::vector<EventCallback>> _callbacks;
    bool _useTimeChecks = false;
    shared::TimePoint _timeThreshold;
};

}