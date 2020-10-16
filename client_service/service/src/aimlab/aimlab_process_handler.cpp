#include "aimlab/aimlab_process_handler.h"

#include "shared/filesystem/common_paths.h"
#include "shared/time.h"
#include "shared/log/log.h"
#include "api/squadov_api.h"
#include "database/api.h"
#include "local/local_data.h"
#include "recorder/game_recorder.h"
#include "game_event_watcher/aimlab/aimlab_log_watcher.h"

#include <atomic>
#include <iostream>
#include <unordered_set>

using namespace std::chrono_literals;
namespace fs = std::filesystem;

namespace service::aimlab {

class AimlabProcessHandlerInstance {
public:
    AimlabProcessHandlerInstance(const process_watcher::process::Process& p, const service::database::DatabaseApi* db);
    ~AimlabProcessHandlerInstance();

private:
    void onAimlabTaskStart(const shared::TimePoint& eventTime, const void* rawData);
    void onAimlabTaskKill(const shared::TimePoint& eventTime, const void* rawData);
    void onAimlabTaskRestart(const shared::TimePoint& eventTime, const void* rawData);
    void onAimlabTaskFinish(const shared::TimePoint& eventTime,const void* rawData);

    void backfill();

    game_event_watcher::AimlabLogWatcherPtr _logWatcher;
    const service::database::DatabaseApi* _db;
    process_watcher::process::Process  _process;

    service::recorder::GameRecorderPtr _recorder;
    AimlabDbInterfacePtr _aimlab;
    shared::TimePoint _taskStartTime;
};

AimlabProcessHandlerInstance::AimlabProcessHandlerInstance(const process_watcher::process::Process& p, const service::database::DatabaseApi* db):
    _logWatcher(new game_event_watcher::AimlabLogWatcher),
    _db(db),
    _process(p),
    _aimlab(new AimlabDbInterface(shared::filesystem::getAimlabAppDataFolder() / fs::path("klutch.bytes"))) {

    _recorder = std::make_unique<service::recorder::GameRecorder>(_process, shared::EGame::Aimlab);
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EAimlabLogEvents::StartTask), std::bind(&AimlabProcessHandlerInstance::onAimlabTaskStart, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EAimlabLogEvents::KillTask), std::bind(&AimlabProcessHandlerInstance::onAimlabTaskKill, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EAimlabLogEvents::RestartTask), std::bind(&AimlabProcessHandlerInstance::onAimlabTaskRestart, this, std::placeholders::_1, std::placeholders::_2));
    _logWatcher->notifyOnEvent(static_cast<int>(game_event_watcher::EAimlabLogEvents::FinishTask), std::bind(&AimlabProcessHandlerInstance::onAimlabTaskFinish, this, std::placeholders::_1, std::placeholders::_2));

    backfill();
}

AimlabProcessHandlerInstance::~AimlabProcessHandlerInstance() {
}

void AimlabProcessHandlerInstance::backfill() {
    // We need to pass the data inside Aim Lab's database to the API server; however, we want to make sure
    // we're communicating as little information as possible. I do not believe we can use the Aim Lab database's
    // ID as a sure-fire way of determining whether or not a change has happened as I believe I have seen
    // smaller IDs for more recent tasks. Thus, every time we backfill (aka every time Aim Lab is open), we
    // send all data since the last time we backfilled using the *date time* of the task. We track the last backfill
    // time in our SQLite database since it already exists. We pull all the task data and then just fire off
    // everything to the server and leave it to the server to filter out stuff that already exists.
    const shared::TimePoint lastBackfillTime = service::local::getLocalData()->getLastAimlabBackfillTime();
    std::vector<shared::aimlab::TaskData> tasksToBackfill = _aimlab->getAllTaskDataSince(lastBackfillTime);

    // We don't want to send too much data at a time so split this vector into chunks and send them one by one.
    constexpr size_t maxChunkSize = 100;
    for (size_t i = 0; i < tasksToBackfill.size(); i+= maxChunkSize) {
        const bool atEnd = (i + maxChunkSize) >= tasksToBackfill.size();
        std::vector<shared::aimlab::TaskData> tasksToSend(tasksToBackfill.begin() + i, atEnd ? tasksToBackfill.end() : (tasksToBackfill.begin() + i + maxChunkSize));
        try {
            service::api::getGlobalApi()->bulkUploadAimlabTasks(tasksToSend);
        } catch (std::exception& ex) {
            std::cerr << "Aim Lab Backfill Failure...Delaying: " << ex.what() << std::endl;
            break;
        }

        // A lot of disk IO here ... but hopefully just a one time thing so not too bad :D
        service::local::getLocalData()->markAimlabBackfillTime(tasksToSend.back().createDate);
    }
}

void AimlabProcessHandlerInstance::onAimlabTaskStart(const shared::TimePoint& eventTime, const void* rawData) {
    const auto* state = reinterpret_cast<const game_event_watcher::AimlabLogState*>(rawData);
    LOG_INFO("[" << shared::timeToStr(eventTime) << "] Aim Lab Task Start" << std::endl
        << "\tTask: " << state->taskName << " " << state->taskMode << std::endl
        << "\tMap: " << state->taskMap << std::endl
        << "\tVersion: " << state->gameVersion << std::endl);

    _recorder->start();
    _taskStartTime = eventTime;
}

void AimlabProcessHandlerInstance::onAimlabTaskKill(const shared::TimePoint& eventTime, const void* rawData) {
    const auto* state = reinterpret_cast<const game_event_watcher::AimlabLogState*>(rawData);
    LOG_INFO("[" << shared::timeToStr(eventTime) << "] Aim Lab Task Kill" << std::endl
        << "\tTask: " << state->taskName << " " << state->taskMode << std::endl
        << "\tMap: " << state->taskMap << std::endl
        << "\tVersion: " << state->gameVersion << std::endl);

    if (_recorder->isRecording()) {
        const auto vodId = _recorder->currentId();
        _recorder->stop();
        service::api::getGlobalApi()->deleteVod(vodId.videoUuid);
    }
}

void AimlabProcessHandlerInstance::onAimlabTaskRestart(const shared::TimePoint& eventTime, const void* rawData) {
    const auto* state = reinterpret_cast<const game_event_watcher::AimlabLogState*>(rawData);
    LOG_INFO("[" << shared::timeToStr(eventTime) << "] Aim Lab Task Restart" << std::endl
        << "\tTask: " << state->taskName << " " << state->taskMode << std::endl
        << "\tMap: " << state->taskMap << std::endl
        << "\tVersion: " << state->gameVersion << std::endl);

    // Stop recording. Delete the old video. Start recording again.
    onAimlabTaskKill(eventTime, rawData);
    onAimlabTaskStart(eventTime, rawData);
}

void AimlabProcessHandlerInstance::onAimlabTaskFinish(const shared::TimePoint& eventTime,const void* rawData) {
    const auto* state = reinterpret_cast<const game_event_watcher::AimlabLogState*>(rawData);
    LOG_INFO("[" << shared::timeToStr(eventTime) << "] Aim Lab Task Finish" << std::endl
        << "\tTask: " << state->taskName << " " << state->taskMode << std::endl
        << "\tMap: " << state->taskMap << std::endl
        << "\tVersion: " << state->gameVersion << std::endl);

    // Pull record of the latest task done from AimLab's database and
    // associate that in our database with the recorded video. Also store
    // a copy in our own database.
    if (_recorder->isRecording()) {
        const auto vodId = _recorder->currentId();
        _recorder->stop();

        // It might take a few tries to grab the data from the SQLite database.
        // I'm assuming it's because Aim Lab will write exclusively to the database
        // and if we try to read from the databse during time it'll fail.
        bool success = false;
        for (auto i = 0; i < 10; ++i) {
            shared::aimlab::TaskData lastData;
            try {
                lastData = _aimlab->getLatestTaskData();
                LOG_INFO("Pulled Data [" << lastData.taskName << " " << lastData.mode << "] - " << lastData.score << std::endl);
            } catch (...) {
                // Don't retry immediately. Wait 10 seconds in total and hopefully
                // we'll get through....
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            _db->storeAimlabTask(lastData, "");
            const auto matchUuid = service::api::getGlobalApi()->uploadAimlabTask(lastData);

            shared::squadov::VodAssociation association;
            association.matchUuid = matchUuid;
            association.userUuid = vodId.userUuid;
            association.videoUuid = vodId.videoUuid;
            association.startTime = _taskStartTime;
            association.endTime = eventTime;
            service::api::getGlobalApi()->associateVod(association);
            success = true;
            break;
        }

        if (!success) {
            // Failed to pull data - hopefully we never get here but just remove the video.
            service::api::getGlobalApi()->deleteVod(vodId.videoUuid);
        }
    }
}

AimlabProcessHandler::AimlabProcessHandler(const service::database::DatabaseApi* db):
    _db(db) {    
}

AimlabProcessHandler::~AimlabProcessHandler() {
}

void AimlabProcessHandler::onProcessStarts(const process_watcher::process::Process& p) {
    // Aimlab already started - can ignore.
    if (_instance) {
        return;
    }

    LOG_INFO("START AIMLAB" << std::endl);
    _instance = std::make_unique<AimlabProcessHandlerInstance>(p, _db);
}

void AimlabProcessHandler::onProcessStops() {
    if (!_instance) {
        return;
    }
    
    LOG_INFO("STOP AIMLAB" << std::endl);
    _instance.reset(nullptr);
}

}