#include "aimlab/aimlab_process_handler.h"

#include "shared/filesystem/common_paths.h"
#include "shared/time.h"
#include "shared/log/log.h"
#include "database/api.h"
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
    // We just need to get a list of tasks in the database that we haven't already processed while being somewhat efficient.
    // Make two database calls
    //  1) To get the N tasks stored in the Aim Lab database.
    //  2) To get the M tasks stored in our database.
    // We need to check which of the N tasks we haven't already stored in the database.
    // We want this to be fairly efficient for a large number of tasks (N/M) so we can use an algorithm similar to
    // how we'd merge two lists.
    // ASSUMPTION: Both databases return the tasks sorted by task id in reverse order. The M tasks are a
    //             subset of the N tasks so M <= N.
    // RUNTIME: O(N + M)
    // INPUTS: nTasks (array of task ids in aim lab database), mTasks (array of task ids in our database)
    // ALGORITHM:
    //  1) n = 0, m = 0
    //  2a) If nTasks[n] == mTasks[m]: n++, m++ 
    //      Explanation: Both lists have the same element, don't have to backfill this one.
    //  2b) If nTasks[n] > mTasks[m]: BACKFILL(nTasks[n]), n++ 
    //      Explanation: The Aim Lab database found a task that we don't have. We know for a fact that
    //                   the element can't be found later in mTasks because mTasks is sorted in reverse order
    //                   so any indices > m would be strictly lower than mTasks[m]. Thus, this task doesn't exist
    //                   in mTasks.
    //  2c) If nTasks[n] < mTasks[m]: ERROR.
    //      Explanation: This means we have a task in our database that doesn't exist in the Aim Lab database. This
    //                   is an error.
    std::vector<int> nTasks = _aimlab->getAllTaskDataId();
    std::vector<int> mTasks = _db->allStoredAimlabTaskIds();
    
    size_t n = 0;
    size_t m = 0;

    while (n < nTasks.size()) {
        if (m >= mTasks.size() || nTasks[n] > mTasks[m]) {
            LOG_INFO("Backfilling Aim Lab Task: " << nTasks[n] << std::endl);
            const auto taskData = _aimlab->getTaskDataFromId(nTasks[n]);
            _db->storeAimlabTask(taskData, "");
            ++n;
        } else if (nTasks[n] == mTasks[m]) {
            ++n;
            ++m;
            continue;
        } else {
            LOG_WARNING("AIM LAB BACKFILL ERROR. UNKNOWN TASK: " << mTasks[m] << std::endl);
        }
    }
}

void AimlabProcessHandlerInstance::onAimlabTaskStart(const shared::TimePoint& eventTime, const void* rawData) {
    const auto* state = reinterpret_cast<const game_event_watcher::AimlabLogState*>(rawData);
    LOG_INFO("[" << shared::timeToStr(eventTime) << "] Aim Lab Task Start" << std::endl
        << "\tTask: " << state->taskName << " " << state->taskMode << std::endl
        << "\tMap: " << state->taskMap << std::endl
        << "\tVersion: " << state->gameVersion << std::endl);

    // Need to generate a unique filename since we don't actually ahve the task ID that AimLab will store at this point.
    std::ostringstream fname;
    fname << state->taskName << "-" << state->taskMode << "-" << shared::fnameTimeToStr(eventTime);
    _recorder->start(fname.str());
}

void AimlabProcessHandlerInstance::onAimlabTaskKill(const shared::TimePoint& eventTime, const void* rawData) {
    const auto* state = reinterpret_cast<const game_event_watcher::AimlabLogState*>(rawData);
    LOG_INFO("[" << shared::timeToStr(eventTime) << "] Aim Lab Task Kill" << std::endl
        << "\tTask: " << state->taskName << " " << state->taskMode << std::endl
        << "\tMap: " << state->taskMap << std::endl
        << "\tVersion: " << state->gameVersion << std::endl);

    if (_recorder->isRecording()) {
        //const auto oldPath = _recorder->path();
        _recorder->stop();
        //std::filesystem::remove(oldPath);
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
        // const auto path = _recorder->path();
        _recorder->stop();

        // It might take a few tries to grab the data from the SQLite database.
        // I'm assuming it's because Aim Lab will write exclusively to the database
        // and if we try to read from the databse during time it'll fail.
        bool success = false;
        for (auto i = 0; i < 10; ++i) {
            try {
                const auto lastData = _aimlab->getLatestTaskData();
                LOG_INFO("Pulled Data [" << lastData.taskName << " " << lastData.mode << "] - " << lastData.score << std::endl);

                // If we weren't recording that means the task was already killed.
                _db->storeAimlabTask(lastData, "");
                success = true;
                break;
            } catch (...) {
                // Don't retry immediately. Wait 10 seconds in total and hopefully
                // we'll get through....
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        if (!success) {
            // Failed to pull data - hopefully we never get here but just remove the video.
            // std::filesystem::remove(path);
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