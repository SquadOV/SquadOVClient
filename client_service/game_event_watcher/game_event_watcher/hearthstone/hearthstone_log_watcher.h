#pragma once

#include "game_event_watcher/logs/base_log_watcher.h"
namespace game_event_watcher {

// The hearthstone is just an amalgamation of a few different log watchers, namely:
// 1) The "default" log (log entries without a [XYZ] prefix).
// 2) The "Decks" log - one way to allow us to detect the current deck.
// 3) The "Power" log - the way for us to detect what happens in game.
class HearthstoneLogWatcher: public BaseLogWatcher {
public:
    static void enableHearthstoneLogging();

    HearthstoneLogWatcher();
};

using HearthstoneLogWatcherPtr = std::unique_ptr<HearthstoneLogWatcher>;

}