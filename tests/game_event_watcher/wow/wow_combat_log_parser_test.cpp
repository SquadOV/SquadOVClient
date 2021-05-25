#define BOOST_TEST_MODULE WoW Combat Log Parsing Test
#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/preprocessor.hpp>

#include <date/tz.h>
#include <filesystem>
#include "game_event_watcher/wow/wow_log_watcher.h"
#include "shared/time.h"
#include "shared/log/log.h"
#include "shared/filesystem/common_paths.h"

using namespace game_event_watcher;
namespace fs = std::filesystem;

BOOST_AUTO_TEST_CASE(comma_in_token_test)
{
    shared::log::Log::initializeGlobalLogger("test.log");
    date::set_install(fs::path(BOOST_PP_STRINGIZE(SQUADOV_TZDATA)));

    WoWLogWatcher watcher(false, shared::nowUtc());
    bool found = false;
    watcher.notifyOnEvent(static_cast<int>(EWoWLogEvents::EncounterStart), [&found](const shared::TimePoint& tm, const void* data){
        const auto* dd = reinterpret_cast<const WoWEncounterStart*>(data);
        BOOST_TEST(dd->encounterId == 2395);
        BOOST_TEST(dd->encounterName == "Hakkar, the Soulflayer");
        BOOST_TEST(dd->difficulty == 23);
        BOOST_TEST(dd->numPlayers == 5);
        BOOST_TEST(dd->instanceId == 2291);

        found = true;
    });
    
    const auto pth = fs::path(BOOST_PP_STRINGIZE(DATA_FOLDER)) / fs::path("comma_in_token.txt");
    watcher.loadFromPath(pth, false);
    watcher.wait();
    BOOST_TEST(found);
}