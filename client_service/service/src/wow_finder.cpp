#include "process_watcher/process/process.h"
#include "process_watcher/games/wow_process_detector.h"
#include "system/win32/hwnd_utils.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace process_watcher;

int main(int argc, char** argv) {
    std::vector<process::Process> processes;
    process::listRunningProcesses(processes);

    games::WoWProcessDetector detector;
    size_t idx = 0;
    const bool running = detector.checkIsRunning(processes, &idx);
    if (running) {
        std::cout << "FOUND RUNNING WOW" << std::endl;
    } else {
        std::cout << "DID NOT FIND WOW RUNNING" << std::endl;
        return 1;
    }

    std::cout << "FOUND PROCESS: " << processes[idx].path() << "\t" << processes[idx].pid() << std::endl;
    HWND wnd = service::system::win32::findWindowForProcessWithMaxDelay(processes[idx].pid(), std::chrono::milliseconds(120000));

    char text[300];
    GetWindowTextA(wnd, text, 300);

    std::cout << "Window: " << wnd << " " << text << std::endl;

    for (int i = 0; i < 100; ++i) {
        if (!IsIconic(wnd)) {
            HMONITOR refMonitor = MonitorFromWindow(wnd, MONITOR_DEFAULTTOPRIMARY);
            std::cout << "Monitor: " << refMonitor << std::endl;
            RECT windowRes;
            GetClientRect(wnd, &windowRes);

            std::cout << windowRes << std::endl;
            std::cout << "Width: " << windowRes.right - windowRes.left << std::endl;
            std::cout << "Height: " << windowRes.bottom - windowRes.top << std::endl;
            std::cout << "Window: " << !service::system::win32::isFullscreen(wnd, refMonitor, 4) << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}