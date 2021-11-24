#include "system/processes.h"
#include "shared/system/win32/interfaces/win32_system_process_interface.h"

#include <unordered_set>

namespace service::system {
namespace {

BOOL enumWindowCallback(HWND hwnd, LPARAM param) {
    auto* data = (std::unordered_set<OSPID>*)param;

    DWORD refPid;
    GetWindowThreadProcessId(hwnd, &refPid);
    if (IsWindowEnabled(hwnd) && IsWindowVisible(hwnd)) {
        data->insert(refPid);
    }
    return TRUE;
}

}

std::vector<process_watcher::process::ProcessRecord> getListOfUserFacingProcesses() {
    auto itf = std::make_shared<shared::system::win32::interfaces::Win32SystemProcessInterface>();
    process_watcher::process::ProcessRunningState processState(itf);
    processState.update();

    // We need to filter these processes and check if they have some valid window.
    // This way we only present user-facing apps to the user rather than all the System32 apps that the user will never need to know are running.
    std::unordered_set<OSPID> validProcesses;
    EnumWindows(enumWindowCallback, (LPARAM)&validProcesses);

    auto list = processState.getList();
    list.erase(std::remove_if(list.begin(), list.end(), [&validProcesses](const auto& x){
        return (validProcesses.find(x.pid) == validProcesses.end()) || x.fullPath.empty();
    }), list.end());

    // Do a second pass to only get unique elements based on the EXE name to make it less confusing for users
    // since the EXE is what we'll be using at the end of the day to find the proper process later on.
    std::unordered_set<std::string> uniqueProcesses;
    std::vector<process_watcher::process::ProcessRecord> finalList;
    finalList.reserve(list.size());

    for (const auto& l: list) {
        if (uniqueProcesses.find(l.exe) == uniqueProcesses.end()) {
            finalList.push_back(l);
            uniqueProcesses.insert(l.exe);
        }
    }

    // Only get icons for the ones we care to show.
    for (auto& p: finalList) {
        p.ico = itf->getBase64EncodedIconForExe(std::filesystem::path(p.fullPath));
    }

    return finalList;
}

}