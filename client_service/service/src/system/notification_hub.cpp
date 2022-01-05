#include "system/notification_hub.h"
#include "shared/errors/error.h"

namespace service::system {

NotificationHub::NotificationHub() {
}

NotificationHub* getNotificationHub() {
    static auto singleton = std::make_unique<NotificationHub>();
    return singleton.get();
}

void NotificationHub::addNotification(NotificationSeverity severity, NotificationDisplayType display, const std::string& title, const std::string& message) {
    NotificationMessage m;
    m.severity = severity;
    m.display = display;
    m.title = title;
    m.message = message;
    m.tm = shared::nowUtc();
    addNotification(m);
}

void NotificationHub::addNotification(const NotificationMessage& m) {
    for (const auto& cb: _callbacks) {
        cb(m);
    }
}

void NotificationHub::addCallback( const NotificationHubCallback& cb) {
    _callbacks.push_back(cb);
}

}