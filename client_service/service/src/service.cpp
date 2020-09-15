#include "process_watcher/watcher.h"
#include "shared/games.h"
#include "valorant/valorant_process_handler.h"

#include <chrono>
#include <thread>

int main(int argc, char** argv) {
    // Start process watcher to watch for our supported games.
    process_watcher::ProcessWatcher watcher;
    watcher.beginWatchingGame(shared::EGame::Valorant, std::make_unique<service::valorant::ValorantProcessHandler>());
    watcher.start();
}