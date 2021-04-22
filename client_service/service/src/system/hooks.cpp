#include "system/hooks.h"
#include "shared/errors/error.h"
#include <memory>
#include <string>
#include <Windows.h>

namespace service::system {
namespace {

class GlobalHookManager {
public:
    static GlobalHookManager* singleton();

    void initialize();
    void cleanup();
private:
    HINSTANCE _dll64 = nullptr;
    HHOOK _khook64 = nullptr;
    HHOOK _mhook64 = nullptr;
};

using GlobalHookManagerPtr = std::unique_ptr<GlobalHookManager>;

}

GlobalHookManager* GlobalHookManager::singleton() {
    static auto singleton = std::make_unique<GlobalHookManager>();
    return singleton.get();
}

void GlobalHookManager::initialize() {
    const auto dllName = "libinputhooks.dll";
    _dll64 = LoadLibrary(dllName);
    if (!_dll64) {
        THROW_ERROR("Failed to load hook DLL: " << dllName);
    }


    HOOKPROC kproc = (HOOKPROC)GetProcAddress(_dll64, "squadOvKeyboardProc");
    if (!kproc) {
        THROW_ERROR("Failed to get DLL proc address [squadOvKeyboardProc]");
    }
    _khook64 = SetWindowsHookEx(WH_KEYBOARD_LL, kproc, _dll64, 0);
    if (!_khook64) {
        THROW_ERROR("Failed to set windows keyboard hook!");
    }

    HOOKPROC mproc = (HOOKPROC)GetProcAddress(_dll64, "squadOvMouseProc");
    if (!mproc) {
        THROW_ERROR("Failed to get DLL proc address [squadOvMouseProc]");
    }
    _mhook64 = SetWindowsHookEx(WH_MOUSE_LL, mproc, _dll64, 0);
    if (!_mhook64) {
        THROW_ERROR("Failed to set windows mouse hook!");
    }
}

void GlobalHookManager::cleanup() {
    if (_khook64) {
        UnhookWindowsHookEx(_khook64);
    }

    if (_mhook64) {
        UnhookWindowsHookEx(_mhook64);
    }
}

void initializeGlobalHooks() {
    GlobalHookManager::singleton()->initialize();
}

void shutdownGlobalHooks() {
    GlobalHookManager::singleton()->cleanup();
}

}