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
    HHOOK _hook64 = nullptr;

    void loadHookDll(const std::string& dllName, HINSTANCE& dll, HHOOK& hook);
};

using GlobalHookManagerPtr = std::unique_ptr<GlobalHookManager>;

}

GlobalHookManager* GlobalHookManager::singleton() {
    static auto singleton = std::make_unique<GlobalHookManager>();
    return singleton.get();
}

void GlobalHookManager::initialize() {
    loadHookDll("libinputhooks.dll", _dll64, _hook64);
}

void GlobalHookManager::loadHookDll(const std::string& dllName, HINSTANCE& dll, HHOOK& hook) {
    dll = LoadLibrary(dllName.c_str());
    if (!dll) {
        THROW_ERROR("Failed to load hook DLL: " << dllName);
    }
    HOOKPROC proc = (HOOKPROC)GetProcAddress(dll, "squadOvKeyboardProc");
    if (!proc) {
        THROW_ERROR("Failed to get DLL proc address [squadOvKeyboardProc]");
    }
    hook = SetWindowsHookEx(WH_KEYBOARD_LL, proc, dll, 0);
    if (!hook) {
        THROW_ERROR("Failed to set windows keyboard hook!");
    }
}

void GlobalHookManager::cleanup() {
    if (_hook64) {
        UnhookWindowsHookEx(_hook64);
    }
}

void initializeGlobalHooks() {
    GlobalHookManager::singleton()->initialize();
}

void shutdownGlobalHooks() {
    GlobalHookManager::singleton()->cleanup();
}

}