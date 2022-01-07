#pragma once

#include "shared/time.h"
#include "shared/json.h"
#include <memory>
#include <sstream>
#include <functional>
#include <vector>

namespace service::system {

enum class NotificationSeverity {
    Warning,
    Error
};

enum class NotificationDisplayType {
    NativeNotification
};

struct NotificationMessage {
    NotificationSeverity severity;
    NotificationDisplayType display;
    std::string title;
    std::string message;
    shared::TimePoint tm;
};

using NotificationHubCallback = std::function<void(const NotificationMessage&)>;

// A way to communicate errors/warnings that happen within the client service
// back up to the user's UI.
class NotificationHub {
public:
    NotificationHub();

    void addNotification(NotificationSeverity severity, NotificationDisplayType display, const std::string& title, const std::string& message);
    void addNotification(const NotificationMessage& m);
    void addCallback(const NotificationHubCallback& cb);

private:
    std::vector<NotificationHubCallback> _callbacks;
};
using NotificationHubPtr = std::unique_ptr<NotificationHub>;

NotificationHub* getNotificationHub();

}

#define DISPLAY_NOTIFICATION(severity, display, title, message) { \
    std::ostringstream tstr; \
    tstr << title; \
    std::ostringstream mstr; \
    mstr << message; \
    service::system::getNotificationHub()->addNotification(severity, display, tstr.str(), mstr.str()); \
}

namespace shared::json {

template<>
struct JsonConverter<service::system::NotificationMessage> {
    static nlohmann::json to(const service::system::NotificationMessage& v) {
        const nlohmann::json data = {
            { "severity", static_cast<int>(v.severity) },
            { "display", static_cast<int>(v.display) },
            { "title", v.title },
            { "message", v.message },
            { "tm", shared::timeToIso(v.tm) }
        };
        return data;
    }
};

}