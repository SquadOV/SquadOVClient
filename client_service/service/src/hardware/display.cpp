#include "hardware/display.h"
#include "shared/strings/strings.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"

#ifdef _WIN32
#include <Windows.h>
#include <d3d11.h>
#include <d3d11_1.h>
#pragma comment(lib, "dxgi")
#endif

namespace service::hardware {
namespace {

const GUID GUID_CLASS_MONITOR = {0x4d36e96e, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18};

}

Display getDisplay() {
    Display display;

#ifdef _WIN32
    IDXGIFactory1* factory = nullptr;
    IDXGIAdapter1* adapter = nullptr;
    IDXGIOutput* dxgiOutput = nullptr;

    HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&factory));
    if (hr != S_OK) {
        LOG_WARNING("...Failed to get display due to DXGI factory failure." << std::endl);
        return display;
    }

    UINT lastAdapterIndex = 0;
    while (factory->EnumAdapters1(lastAdapterIndex++, &adapter) != DXGI_ERROR_NOT_FOUND) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        std::wstring strDesc(desc.Description);
        GPUInfo gpu;
        gpu.name = shared::strings::wcsToUtf8(std::wstring(desc.Description));
        gpu.memoryBytes = desc.DedicatedVideoMemory;

        if (gpu.name == "Microsoft Basic Render Driver") {
            adapter->Release();
            continue;
        }
        display.gpus.push_back(gpu);

        adapter->Release();
    }

    if (adapter) {
        adapter->Release();
    }
    factory->Release();
#endif
    return display;
}

nlohmann::json Monitor::toJson() const {
    const nlohmann::json obj = {
        { "manufacturer", manufacturer },
        { "name", name },
        { "width", width },
        { "height", height },
        { "year", year },
        { "refresh", refresh }
    };
    return obj;
}

nlohmann::json GPUInfo::toJson() const {
    const nlohmann::json obj = {
        { "name", name },
        { "memoryBytes", memoryBytes }
    };
    return obj;
}

nlohmann::json Display::toJson() const {
    nlohmann::json arr = {
        { "gpus", nlohmann::json::array() },
        { "monitors", nlohmann::json::array() }
    };

    for (const auto& p : gpus) {
        arr["gpus"].push_back(p.toJson());
    }

    for (const auto& p : monitors) {
        arr["monitors"].push_back(p.toJson());
    }

    return arr;
}

std::ostream& operator<<(std::ostream& os, const Display& p) {
    os << "GPUs:" << std::endl;
    for (const auto& g : p.gpus) {
        os << g << std::endl;
    }
    os << "Monitors:" << std::endl;
    for (const auto& g : p.monitors) {
        os << g << std::endl;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Monitor& p) {
    os << " Name: " << p.manufacturer << " " << p.name << "(" << p.year << ")" << std::endl
       << " Resolution: " << p.width << "x" << p.height << std::endl
       << " Refresh Rate: " << p.refresh << "Hz";
    return os;
}

std::ostream& operator<<(std::ostream& os, const GPUInfo& p) {
    os << " Name: " << p.name << std::endl
       << " Memory: " << p.memoryBytes << " bytes";
    return os;
}

}