#define BOOST_TEST_MODULE Process Running State Test
#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/preprocessor.hpp>

#include <date/tz.h>
#include <filesystem>
#include <unordered_map>

#include "shared/time.h"
#include "shared/log/log.h"
#include "process_watcher/process/process.h"
#include "shared/system/interfaces/system_process_interface.h"

struct TestDatum {
    OSPID pid;
    OSString name;
    bool hasWindow;
};

class TestSystemProcessInterface: public shared::system::SystemProcessInterface {
public:
    explicit TestSystemProcessInterface(const std::vector<TestDatum>& data) {
        updateProcesses(data);
    }

    void updateProcesses(const std::vector<TestDatum>& data) {
        _pidMap.clear();
        _nameMap.clear();
        addProcesses(data);
    }

    void addProcesses(const std::vector<TestDatum>& data) {
        for (const auto& d : data) {
            _pidMap[d.pid] = d;
            _nameMap[d.name] = d;
        }
    }

    std::vector<OSPID> enumProcesses() const override {
        std::vector<OSPID> ret;
        ret.reserve(_pidMap.size());

        for (const auto& d : _pidMap) {
            ret.push_back(d.first);
        }
        return ret;
    }

    OSString getProcessName(OSPID pid) const override {
        return _pidMap.at(pid).name;
    }

    OSWindow findWindowForProcessWithMaxDelay(OSPID pid, const std::chrono::milliseconds& maxDelayMs, const std::chrono::milliseconds& step, bool quiet, bool checkWindowSize) const override{
        const auto p = _pidMap.at(pid);
        if (p.hasWindow) {
            return reinterpret_cast<OSWindow>(1);
        } else {
            return nullptr;
        }
    }

    OSString getProcessFriendlyName(const std::filesystem::path& path) const override { return L""; }
    std::string getBase64EncodedIconForExe(const std::filesystem::path& path) const override { return ""; }

private:
    std::unordered_map<OSPID, TestDatum> _pidMap;
    std::unordered_map<OSString, TestDatum> _nameMap;
};

using namespace process_watcher;
namespace fs = std::filesystem;

BOOST_AUTO_TEST_CASE(find_process_by_pid)
{
    shared::log::Log::initializeGlobalLogger("test.log");
    date::set_install(fs::path(BOOST_PP_STRINGIZE(SQUADOV_TZDATA)));

    std::vector<TestDatum> referenceData = {
        { 101, L"chicken", true }
    };

    process::ProcessRunningState state(std::make_shared<TestSystemProcessInterface>(referenceData));
    state.update();

    for (const auto& d: referenceData) {
        {
            const auto p = state.getProcesssRunningByPid(d.pid, true);
            BOOST_TEST(p.has_value());
            BOOST_TEST(p.value().pid() == d.pid);
            BOOST_CHECK(p.value().name() == d.name);
        }

        {
            const auto p = state.getProcesssRunningByPid(d.pid, false);
            BOOST_TEST(p.has_value());
            BOOST_TEST(p.value().pid() == d.pid);
            BOOST_CHECK(p.value().name() == d.name);
        }
    }
}

BOOST_AUTO_TEST_CASE(find_process_by_pid_no_window)
{
    shared::log::Log::initializeGlobalLogger("test.log");
    date::set_install(fs::path(BOOST_PP_STRINGIZE(SQUADOV_TZDATA)));

    std::vector<TestDatum> referenceData = {
        { 101, L"chicken", false }
    };

    process::ProcessRunningState state(std::make_shared<TestSystemProcessInterface>(referenceData));
    state.update();

    for (const auto& d: referenceData) {
        {
            const auto p = state.getProcesssRunningByPid(d.pid, true);
            BOOST_TEST(!p.has_value());
        }

        {
            const auto p = state.getProcesssRunningByPid(d.pid, false);
            BOOST_TEST(p.has_value());
            BOOST_TEST(p.value().pid() == d.pid);
            BOOST_CHECK(p.value().name() == d.name);
        }
    }
}

BOOST_AUTO_TEST_CASE(find_process_by_name)
{
    shared::log::Log::initializeGlobalLogger("test.log");
    date::set_install(fs::path(BOOST_PP_STRINGIZE(SQUADOV_TZDATA)));

    std::vector<TestDatum> referenceData = {
        { 101, L"chicken", true }
    };

    process::ProcessRunningState state(std::make_shared<TestSystemProcessInterface>(referenceData));
    state.update();

    for (const auto& d: referenceData) {
        {
            const auto p = state.getProcesssRunningByName(d.name, true);
            BOOST_TEST(!p.empty());
            BOOST_TEST(p[0].pid() == d.pid);
            BOOST_CHECK(p[0].name() == d.name);
        }

        {
            const auto p = state.getProcesssRunningByName(d.name, false);
            BOOST_TEST(!p.empty());
            BOOST_TEST(p[0].pid() == d.pid);
            BOOST_CHECK(p[0].name() == d.name);
        }
    }
}

BOOST_AUTO_TEST_CASE(find_process_by_name_no_window)
{
    shared::log::Log::initializeGlobalLogger("test.log");
    date::set_install(fs::path(BOOST_PP_STRINGIZE(SQUADOV_TZDATA)));

    std::vector<TestDatum> referenceData = {
        { 101, L"chicken", false }
    };

    process::ProcessRunningState state(std::make_shared<TestSystemProcessInterface>(referenceData));
    state.update();

    for (const auto& d: referenceData) {
        {
            const auto p = state.getProcesssRunningByName(d.name, true);
            BOOST_TEST(p.empty());
        }

        {
            const auto p = state.getProcesssRunningByName(d.name, false);
            BOOST_TEST(!p.empty());
            BOOST_TEST(p[0].pid() == d.pid);
            BOOST_CHECK(p[0].name() == d.name);
        }
    }
}

BOOST_AUTO_TEST_CASE(find_process_test_process_drop)
{
    shared::log::Log::initializeGlobalLogger("test.log");
    date::set_install(fs::path(BOOST_PP_STRINGIZE(SQUADOV_TZDATA)));

    std::vector<TestDatum> referenceData = {
        { 101, L"chicken", true }
    };

    auto itf = std::make_shared<TestSystemProcessInterface>(referenceData);

    process::ProcessRunningState state(itf);
    state.update();
    itf->updateProcesses({});
    state.update();

    for (const auto& d: referenceData) {
        BOOST_TEST(state.getProcesssRunningByName(d.name, true).empty());
        BOOST_TEST(state.getProcesssRunningByName(d.name, false).empty());
        BOOST_TEST(!state.getProcesssRunningByPid(d.pid, true));
        BOOST_TEST(!state.getProcesssRunningByPid(d.pid, false));
    }
}

BOOST_AUTO_TEST_CASE(find_process_test_process_add)
{
    shared::log::Log::initializeGlobalLogger("test.log");
    date::set_install(fs::path(BOOST_PP_STRINGIZE(SQUADOV_TZDATA)));

    std::vector<TestDatum> referenceData = {
        { 101, L"chicken", true }
    };

    auto itf = std::make_shared<TestSystemProcessInterface>(referenceData);

    process::ProcessRunningState state(itf);
    state.update();

    referenceData.push_back({
        102, L"bob", true
    });

    itf->updateProcesses(referenceData);
    state.update();

    for (const auto& d: referenceData) {
        BOOST_TEST(!state.getProcesssRunningByName(d.name, true).empty());
        BOOST_TEST(!state.getProcesssRunningByName(d.name, false).empty());
        BOOST_TEST(state.getProcesssRunningByPid(d.pid, true).has_value());
        BOOST_TEST(state.getProcesssRunningByPid(d.pid, false).has_value());
    }
}

// Case 1 is when the PID gets reused and the name changes (or something).
BOOST_AUTO_TEST_CASE(find_process_pid_reuse_1)
{
    shared::log::Log::initializeGlobalLogger("test.log");
    date::set_install(fs::path(BOOST_PP_STRINGIZE(SQUADOV_TZDATA)));

    std::vector<TestDatum> referenceData = {
        { 101, L"chicken", true }
    };

    auto itf = std::make_shared<TestSystemProcessInterface>(referenceData);

    process::ProcessRunningState state(itf);
    state.update();

    for (const auto& d: referenceData) {
        {
            const auto p = state.getProcesssRunningByPid(d.pid, true);
            BOOST_TEST(p.has_value());
            BOOST_TEST(p.value().pid() == d.pid);
            BOOST_CHECK(p.value().name() == d.name);
        }
    }

    referenceData[0] = { 101, L"bob", true };
    itf->updateProcesses(referenceData);
    state.update();

    for (const auto& d: referenceData) {
        {
            const auto p = state.getProcesssRunningByPid(d.pid, true);
            BOOST_TEST(p.has_value());
            BOOST_TEST(p.value().pid() == d.pid);
            BOOST_CHECK(p.value().name() == d.name);
        }
    }
}

// Case 2 is when the PID gets reused after an update where the PID is no longer used.
BOOST_AUTO_TEST_CASE(find_process_pid_reuse_2)
{
    shared::log::Log::initializeGlobalLogger("test.log");
    date::set_install(fs::path(BOOST_PP_STRINGIZE(SQUADOV_TZDATA)));

    std::vector<TestDatum> referenceData = {
        { 101, L"chicken", true }
    };

    auto itf = std::make_shared<TestSystemProcessInterface>(referenceData);

    process::ProcessRunningState state(itf);
    state.update();

    for (const auto& d: referenceData) {
        {
            const auto p = state.getProcesssRunningByPid(d.pid, true);
            BOOST_TEST(p.has_value());
            BOOST_TEST(p.value().pid() == d.pid);
            BOOST_CHECK(p.value().name() == d.name);
        }
    }

    referenceData.clear();
    itf->updateProcesses(referenceData);
    state.update();

    referenceData = {
        { 101, L"bob", true }
    };
    itf->updateProcesses(referenceData);
    state.update();

    for (const auto& d: referenceData) {
        {
            const auto p = state.getProcesssRunningByPid(d.pid, true);
            BOOST_TEST(p.has_value());
            BOOST_TEST(p.value().pid() == d.pid);
            BOOST_CHECK(p.value().name() == d.name);
        }
    }
}