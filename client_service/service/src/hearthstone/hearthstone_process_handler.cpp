#include "hearthstone/hearthstone_process_handler.h"

#include "game_event_watcher/hearthstone/hearthstone_log_watcher.h"
#include "process_watcher/memory/module_memory_mapper.h"
#include "process_watcher/memory/mono_memory_mapper.h"
#include "process_watcher/memory/pe_mapper.h"
#include "process_watcher/memory/games/hearthstone/hearthstone_memory_mapper.h"
#include "shared/log/log.h"
#include "api/squadov_api.h"
#include "recorder/game_recorder.h"

namespace service::hearthstone {
namespace {
const std::string HEARTHSTONE_MONO_MODULE = "mono-2.0-bdwgc.dll";
}

class HearthstoneProcessHandlerInstance {
public:
    HearthstoneProcessHandlerInstance(const process_watcher::process::Process& p);
    ~HearthstoneProcessHandlerInstance();

private:
    void loadMonoMapper(const shared::TimePoint& eventTime, const void* rawData);
    void onGameConnect(const shared::TimePoint& eventTime, const void* rawData);
    void onGameStart(const shared::TimePoint& eventTime, const void* rawData);
    void onGameEnd(const shared::TimePoint& eventTime, const void* rawData);

    process_watcher::process::Process  _process;
    game_event_watcher::HearthstoneLogWatcherPtr _logWatcher;
    process_watcher::memory::games::hearthstone::HearthstoneMemoryMapperPtr _monoMapper;
    service::recorder::GameRecorderPtr _recorder;

    // Track current game information so that we know whether we're in a game or not.
    game_event_watcher::HearthstoneGameConnectionInfo _currentGame;
    std::string _matchUuid;
    bool _inGame = false;
    // This is when the game started from the perspective of the VOD.
    shared::TimePoint _gameStartTime;
    // This is when the game started from the perspective of the log.
    shared::TimePoint _gameStartEventTime;
};

HearthstoneProcessHandlerInstance::HearthstoneProcessHandlerInstance(const process_watcher::process::Process& p):
    _process(p),
    _logWatcher(std::make_unique<game_event_watcher::HearthstoneLogWatcher>(true)) {

    _recorder = std::make_unique<service::recorder::GameRecorder>(_process, shared::EGame::Hearthstone);
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EHearthstoneLogEvents::GameReady), std::bind(&HearthstoneProcessHandlerInstance::loadMonoMapper, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EHearthstoneLogEvents::MatchConnect), std::bind(&HearthstoneProcessHandlerInstance::onGameConnect, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EHearthstoneLogEvents::MatchStart), std::bind(&HearthstoneProcessHandlerInstance::onGameStart, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EHearthstoneLogEvents::MatchEnd), std::bind(&HearthstoneProcessHandlerInstance::onGameEnd, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->loadFromExecutable(p.path());
}

HearthstoneProcessHandlerInstance::~HearthstoneProcessHandlerInstance() {

}

void HearthstoneProcessHandlerInstance::loadMonoMapper(const shared::TimePoint& eventTime, const void* rawData) {
    if (_monoMapper) {
        return;
    }

    LOG_INFO("Hearthstone Game Ready [" << shared::timeToStr(eventTime) << "]" << std::endl);
    // In addition to the log watcher, Hearthstone needs a memory mapper so that we can grab certain data
    // that isn't logged in the file such as the user's/opponent's rank, current deck (kinda), and cards in deck.
    // Note that this function *SHOULD NOT* be called right when the game is started but it should instead wait for an appropriate
    // event from the logs.
    auto memMapper = std::make_shared<process_watcher::memory::ModuleMemoryMapper>(_process, HEARTHSTONE_MONO_MODULE);
    process_watcher::memory::PEMapper peMapper(memMapper);
    auto monoWrapper = std::make_unique<process_watcher::memory::MonoMemoryMapper>(memMapper, peMapper); 
    _monoMapper = std::make_unique<process_watcher::memory::games::hearthstone::HearthstoneMemoryMapper>(std::move(monoWrapper));
}

void HearthstoneProcessHandlerInstance::onGameConnect(const shared::TimePoint& eventTime, const void* rawData) {
    const game_event_watcher::HearthstoneGameConnectionInfo* info = reinterpret_cast<const game_event_watcher::HearthstoneGameConnectionInfo*>(rawData);
    LOG_INFO("Hearthstone Game Connect [" << shared::timeToStr(eventTime) << "]" << std::endl
        << "\tGame Server:" << info->ip << ":" << info->port << std::endl
        << "\tGame ID: " << info->gameId << std::endl
        << "\tClient ID: " << info->clientId << std::endl
        << "\tSpectate Key: " << info->spectateKey << std::endl
        << "\tReconnecting: " << info->reconnecting << std::endl
    );

    // TODO: Handle reconnecting.
    if (info->reconnecting) {
        return;
    }

    // We need to store this information as we'll be using this to identify the game when we upload information about the game soon.
    _currentGame = *info;
    if (_inGame) {
        onGameStart(eventTime, nullptr);
    }
}

void HearthstoneProcessHandlerInstance::onGameStart(const shared::TimePoint& eventTime, const void* rawData) {
    LOG_INFO("Hearthstone Game Start [" << shared::timeToStr(eventTime) << "] - " << _currentGame.valid() << std::endl);

    // Mark this flag just in case the MatchConnect gets fired *after* MatchStart so we know to call onGameStart from the MatchConnect event.
    if (!_inGame) {
        _gameStartEventTime = eventTime;
        _inGame = true;   
    }

    // We need this check between two separate threads are polling the main log file that sends the MatchConnect event
    // and the power log file that sends the MatchStart event so it's theoretically possible to get these events out of order.
    if (!_currentGame.valid()) {
        return;
    }

    // Grab the deck and information about all the players in the game and upload this information first.
    try {
        const auto deck = _monoMapper->getCurrentDeck();
        const auto players = _monoMapper->getCurrentPlayers();
        _matchUuid = service::api::getGlobalApi()->createHearthstoneMatch(_currentGame, *deck, players, _gameStartEventTime);
    } catch (const std::exception& ex) {
        LOG_WARNING("Failed to create Hearthstone match: " << ex.what() << std::endl);
        return;
    }

    _gameStartTime = shared::nowUtc();
    _recorder->start();
}

void HearthstoneProcessHandlerInstance::onGameEnd(const shared::TimePoint& eventTime, const void* rawData) {
    // We do need this check as the game end event happens at every scence change pretty much.
    if (!_inGame) {
        return;
    }

    LOG_INFO("Hearthstone Game End [" << shared::timeToStr(eventTime) << "]" << std::endl);

    _inGame = false;
    _currentGame = game_event_watcher::HearthstoneGameConnectionInfo();

    const auto vodId = _recorder->currentId();
    const auto metadata = _recorder->getMetadata();
    _recorder->stop();

    // Now we need to send the raw power log to the API server for processing as well
    // as tell the server about the association between the VOD we just recorded
    // with the current match.
    try {
        const nlohmann::json* data = reinterpret_cast<const nlohmann::json*>(rawData);
        service::api::getGlobalApi()->uploadHearthstonePowerLogs(_matchUuid, *data);
    } catch (const std::exception& ex) {
        // Not the end of the world - we just won't have match history details - still have a VOD!
        LOG_WARNING("Failed to upload Hearthstone power logs: " << ex.what() << std::endl);
    }

    try {
        shared::squadov::VodAssociation association;
        association.matchUuid = _matchUuid;
        association.userUuid = vodId.userUuid;
        association.videoUuid = vodId.videoUuid;
        association.startTime = _gameStartTime;
        association.endTime = eventTime;
        service::api::getGlobalApi()->associateVod(association, metadata);
    } catch (const std::exception& ex) {
        LOG_WARNING("Failed to associate Hearthstone VOD: " << ex.what() << std::endl);
        service::api::getGlobalApi()->deleteVod(vodId.videoUuid);
    }
}

HearthstoneProcessHandler::HearthstoneProcessHandler() = default;
HearthstoneProcessHandler::~HearthstoneProcessHandler() = default;

void HearthstoneProcessHandler::onProcessStarts(const process_watcher::process::Process& p) {
    if (_instance) {
        return;
    }

    LOG_INFO("START HEARTHSTONE" << std::endl);
    _instance = std::make_unique<HearthstoneProcessHandlerInstance>(p);
}

void HearthstoneProcessHandler::onProcessStops() {
    if (!_instance) {
        return;
    }
    LOG_INFO("STOP HEARTHSTONE" << std::endl);
    _instance.reset(nullptr);
}

}