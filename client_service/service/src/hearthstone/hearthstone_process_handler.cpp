#include "hearthstone/hearthstone_process_handler.h"

#include "game_event_watcher/hearthstone/hearthstone_log_watcher.h"
#include "process_watcher/memory/module_memory_mapper.h"
#include "process_watcher/memory/mono_memory_mapper.h"
#include "process_watcher/memory/pe_mapper.h"
#include "process_watcher/memory/games/hearthstone/hearthstone_memory_mapper.h"
#include "shared/log/log.h"
#include "shared/hearthstone/hearthstone_ratings.h"
#include "api/squadov_api.h"
#include "recorder/game_recorder.h"
#include "system/state.h"

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
    void onGameDisconnect(const shared::TimePoint& eventTime, const void* rawData);

    // Arena Handlers. This will have to be reworked a little bit (probably) if/when we decide
    // to support VOD recording of drafts because we're very much assuming that we can easily
    // support restarting an arena draft.
    void onArenaDraftStart(const shared::TimePoint& eventTime, const void* rawData);
    void onArenaDraftChoice(const shared::TimePoint& eventTime, const void* rawData);
    void onArenaDraftFinish(const shared::TimePoint& eventTime, const void* rawData);

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

    // Arena
    std::string _arenaUuid;

    // In-Game Watcher
    bool _valid = true;
    std::thread _watcherThread;
};

HearthstoneProcessHandlerInstance::HearthstoneProcessHandlerInstance(const process_watcher::process::Process& p):
    _process(p),
    _logWatcher(std::make_unique<game_event_watcher::HearthstoneLogWatcher>(true, shared::nowUtc())) {

    _recorder = std::make_unique<service::recorder::GameRecorder>(_process, shared::EGame::Hearthstone);
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EHearthstoneLogEvents::GameReady), std::bind(&HearthstoneProcessHandlerInstance::loadMonoMapper, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EHearthstoneLogEvents::MatchConnect), std::bind(&HearthstoneProcessHandlerInstance::onGameConnect, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EHearthstoneLogEvents::MatchStart), std::bind(&HearthstoneProcessHandlerInstance::onGameStart, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EHearthstoneLogEvents::MatchEnd), std::bind(&HearthstoneProcessHandlerInstance::onGameEnd, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EHearthstoneLogEvents::MatchDisconnect), std::bind(&HearthstoneProcessHandlerInstance::onGameDisconnect, this, std::placeholders::_1, std::placeholders::_2));

    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EHearthstoneLogEvents::ArenaStartDraft), std::bind(&HearthstoneProcessHandlerInstance::onArenaDraftStart, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EHearthstoneLogEvents::ArenaFinishDraft), std::bind(&HearthstoneProcessHandlerInstance::onArenaDraftFinish, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EHearthstoneLogEvents::ArenaDraftChoice), std::bind(&HearthstoneProcessHandlerInstance::onArenaDraftChoice, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->loadFromExecutable(p.path());
}

HearthstoneProcessHandlerInstance::~HearthstoneProcessHandlerInstance() {
    _inGame = false;
    _valid = false;
    if (_watcherThread.joinable()) {
        _watcherThread.join();
    }
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

void HearthstoneProcessHandlerInstance::onArenaDraftStart(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    if (!_arenaUuid.empty()) {
        LOG_WARNING("Starting an arena draft in the middle of another arena draft?" << std::endl);
        return;
    }

    const int64_t* deckId = reinterpret_cast<const int64_t*>(rawData);
    LOG_INFO("Hearthstone Arena Draft Start [" << shared::timeToStr(eventTime) << " - " << *deckId << "]" << std::endl);
    try {
        _arenaUuid = service::api::getGlobalApi()->createHearthstoneArenaDraft(eventTime, *deckId);
    } catch(std::exception& ex) {
        LOG_WARNING("Failed to start an arena draft: " << ex.what() << std::endl);
    }
}

void HearthstoneProcessHandlerInstance::onArenaDraftChoice(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    if (_arenaUuid.empty()) {
        return;
    }

    const std::string* cardId = reinterpret_cast<const std::string*>(rawData);
    LOG_INFO("Hearthstone Arena Draft Choice [" << shared::timeToStr(eventTime) << " - " << *cardId << "]" << std::endl);
    try {
        service::api::getGlobalApi()->addHearthstoneArenaDraftCard(eventTime, _arenaUuid, *cardId);
    } catch(std::exception& ex) {
        LOG_WARNING("Failed to add arena card choice: " << ex.what() << std::endl);
    }
}

void HearthstoneProcessHandlerInstance::onArenaDraftFinish(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    if (_arenaUuid.empty()) {
        return;
    }

    LOG_INFO("Hearthstone Arena Draft Finish [" << shared::timeToStr(eventTime) << "]" << std::endl);

    try {
        service::api::getGlobalApi()->uploadHearthstoneArenaDeck(*_monoMapper->getCurrentArenaDeck(), _arenaUuid);
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to add upload Arena deck: " << ex.what() << std::endl);
    }
    _arenaUuid = "";
}

void HearthstoneProcessHandlerInstance::onGameConnect(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

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

    // We need to start recording now because the game start event comes much later so if we don't start recording now then we
    // might miss parts of the mulligan.
    _gameStartTime = shared::nowUtc();
    _recorder->start();

    if (_inGame) {
        onGameStart(eventTime, nullptr);
    }
}

void HearthstoneProcessHandlerInstance::onGameStart(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }
    LOG_INFO("Hearthstone Game Start [" << shared::timeToStr(eventTime) << "] - " << _currentGame.valid() << std::endl);

    // We don't want to handle spectate mode as that isn't this user's match.
    // We handle this here because it's likely that isSpecator isn't set to true until
    // later (i.e. not the moment the user connects to the match). In the case that
    // we are spectating we want to immediately stop recording and delete what was already
    // recorded.
    if (_monoMapper->isSpectator()) {
        LOG_INFO("Detecting spectator mode...reverting Hearthstone recording:" << _recorder->isRecording() << std::endl);
        if (_recorder->isRecording()) {
            const auto vodId = _recorder->currentId();
            try {
                service::api::getGlobalApi()->deleteVod(vodId.videoUuid);
            } catch (std::exception& ex) {
                LOG_WARNING("Failed to delete VOD: " << ex.what() << std::endl);
            }
            _recorder->stop();
        }
        _inGame = false;
        _currentGame = game_event_watcher::HearthstoneGameConnectionInfo();
        return;
    }

    // Mark this flag just in case the MatchConnect gets fired *after* MatchStart so we know to call onGameStart from the MatchConnect event.
    if (!_inGame) {
        _gameStartEventTime = eventTime;
        _inGame = true;   
    }

    // We need this check because two separate threads are polling the main log file that sends the MatchConnect event
    // and the power log file that sends the MatchStart event so it's theoretically possible to get these events out of order.
    if (!_currentGame.valid()) {
        return;
    }

    // Grab the deck and information about all the players in the game and upload this information first.
    try {
        LOG_INFO("Retrieve Hearthstone deck from memory." << std::endl);
        const auto deck = _monoMapper->getCurrentDeck();
        LOG_INFO("Retrieve Hearthstone current players from memory." << std::endl);
        const auto players = _monoMapper->getCurrentPlayers();

        LOG_INFO("Retrieve Hearthstone BG ratings from memory." << std::endl);
        shared::hearthstone::HearthstoneRatings ratings;
        ratings.battlegroundsRating = _monoMapper->getBattlegroundsRating();
        LOG_INFO("Retrieve Hearthstone Duels ratings from memory." << std::endl);
        ratings.duelsCasualRating = _monoMapper->getDuelsCasualRating();
        ratings.duelsHeroicRating = _monoMapper->getDuelsHeroicRating();

        _matchUuid = service::api::getGlobalApi()->createHearthstoneMatch(
            _currentGame,
            deck,
            players,
            ratings,
            _gameStartEventTime
        );
    } catch (const std::exception& ex) {
        LOG_WARNING("Failed to create Hearthstone match: " << ex.what() << std::endl);
        return;
    }

    // At this point we need to start a thread to watch for when we're no longer in the game as we can't
    // reliably detect this in the logs.
    _valid = false;
    if (_watcherThread.joinable()) {
        _watcherThread.join();
    }
    _watcherThread = std::thread();
    _valid = true;

    _watcherThread = std::thread([this]() {
        while (_valid) {
            if (!_monoMapper->inGame()) {
                onGameDisconnect(shared::nowUtc(), nullptr);
                break;
            }

            // Precision is not needed here as the only thing we'll pay for is a slightly longer VOD.
            std::this_thread::sleep_for(std::chrono::seconds(200));
        }
    });
}

void HearthstoneProcessHandlerInstance::onGameEnd(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }

    if (!_inGame) {
        return;
    }

    LOG_INFO("Hearthstone Game End [" << shared::timeToStr(eventTime) << "]" << std::endl);

    // This needs to be here instead of "onGameDisconnect". If we set
    // _inGame = false in onGameDisconnect inn the scenario where onGameDisconnect is fired first
    // (due to the two events being parsed from two different logs in two different threads), then
    // we won't send the match logs which is undesirable.
    _inGame = false;
    _currentGame = game_event_watcher::HearthstoneGameConnectionInfo();

    if (!_matchUuid.empty()) {
        // Note that we use the game end event to retrieve the power logs in JSON form and store it for safe keeping.
        // We use the game disconnect event to actually stop recording.
        try {
            const nlohmann::json* data = reinterpret_cast<const nlohmann::json*>(rawData);
            service::api::getGlobalApi()->uploadHearthstonePowerLogs(_matchUuid, *data);
        } catch (const std::exception& ex) {
            // Not the end of the world - we just won't have match history details - still have a VOD!
            LOG_WARNING("Failed to upload Hearthstone power logs: " << ex.what() << std::endl);
        }
    }
}

void HearthstoneProcessHandlerInstance::onGameDisconnect(const shared::TimePoint& eventTime, const void* rawData) {
    if (service::system::getGlobalState()->isPaused()) {
        return;
    }
    
    LOG_INFO("Hearthstone Game Disconnect [" << shared::timeToStr(eventTime) << "]" << std::endl);

    const auto isRecording = _recorder->isRecording();

    if (isRecording) {
        const auto vodId = _recorder->currentId();
        const auto metadata = _recorder->getMetadata();
        const auto sessionId = _recorder->sessionId();

        _recorder->stop();
        // Need this check here just in case for whatever reason we connected to a game but never
        // actually caught the "game start" event.
        try {
            try {
                shared::squadov::VodAssociation association;
                association.matchUuid = _matchUuid;
                association.userUuid = vodId.userUuid;
                association.videoUuid = vodId.videoUuid;
                association.startTime = _gameStartTime;
                association.endTime = eventTime;
                service::api::getGlobalApi()->associateVod(association, metadata, sessionId);
            } catch (const std::exception& ex) {
                LOG_WARNING("Failed to associate Hearthstone VOD: " << ex.what() << std::endl);
                service::api::getGlobalApi()->deleteVod(vodId.videoUuid);
            }
        } catch (std::exception& ex) {
            // This should only be catching delete VOD exceptions which are fine.
            LOG_WARNING("Failed to delete VOD: " << ex.what() << std::endl);
        }
    }
}

HearthstoneProcessHandler::HearthstoneProcessHandler() = default;
HearthstoneProcessHandler::~HearthstoneProcessHandler() = default;

void HearthstoneProcessHandler::onProcessStarts(const process_watcher::process::Process& p) {
    if (_instance) {
        return;
    }

    LOG_INFO("START HEARTHSTONE" << std::endl);
    service::system::getGlobalState()->markGameRunning(shared::EGame::Hearthstone, true);
    _instance = std::make_unique<HearthstoneProcessHandlerInstance>(p);
}

void HearthstoneProcessHandler::onProcessStops() {
    if (!_instance) {
        return;
    }
    LOG_INFO("STOP HEARTHSTONE" << std::endl);
    service::system::getGlobalState()->markGameRunning(shared::EGame::Hearthstone, false);
    _instance.reset(nullptr);
}

}