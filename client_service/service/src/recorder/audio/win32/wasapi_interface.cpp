#include "recorder/audio/win32/wasapi_interface.h"

#ifdef _WIN32
#include "shared/errors/error.h"
#include "shared/log/log.h"
#include "shared/strings/strings.h"

#include <Windows.h>
#include <mmdeviceapi.h>
#include <objbase.h>
#include <propidl.h>
#include <functiondiscoverykeys_devpkey.h>

namespace service::recorder::audio::win32 {

service::recorder::audio::AudioDeviceResponse WASAPIInterface::getDeviceListing(service::recorder::audio::EAudioDeviceDirection dir) {
    AudioDeviceResponse resp;

    IMMDeviceEnumerator* audioEnum = nullptr;
    IMMDeviceCollection* collection = nullptr;
    IMMDevice* device = nullptr;
    LPWSTR deviceId = nullptr;
    IMMDevice* defaultDevice = nullptr;
    LPWSTR defaultDeviceId = nullptr;

    if (dir == EAudioDeviceDirection::Input) {
        LOG_INFO("Listing WASAPI Input Audio Devices..." << std::endl);
    } else {
        LOG_INFO("Listing WASAPI Output Audio Devices..." << std::endl);
    }

    auto safeCleanup = [&](){
        if (audioEnum) {
            audioEnum->Release();
            audioEnum = nullptr;
        }

        if (collection) {
            collection->Release();
            collection = nullptr;
        }

        if (device) {
            device->Release();
            device = nullptr;
        }

        if (deviceId) {
            CoTaskMemFree(deviceId);
            deviceId = nullptr;
        }

        if (defaultDevice) {
            defaultDevice->Release();
            defaultDevice = nullptr;
        }

        if (defaultDeviceId) {
            CoTaskMemFree(defaultDeviceId);
            defaultDeviceId = nullptr;
        }
    };

    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(IMMDeviceEnumerator),
        (LPVOID*)&audioEnum
    );

    if (hr != S_OK) {
        THROW_ERROR("Failed to create IMMDeviceEnumerator: " << hr);
    }

    const auto dataflow = (dir == service::recorder::audio::EAudioDeviceDirection::Output) ? eRender : eCapture;

    hr = audioEnum->GetDefaultAudioEndpoint(
        dataflow,
        eConsole,
        &defaultDevice
    );

    if (hr != S_OK) {
        if (hr == E_NOT_SET) {
            // In this case we did not find a default device. Which is fine - the rest
            // of the code doesn't necessarily depend on finding the default device.
            LOG_INFO("...No default device found." << std::endl);
            defaultDevice = nullptr;
        } else {
            safeCleanup();
            THROW_ERROR("Failed to get default audio endpoint: " << hr);
        }
    } else {
        hr = defaultDevice->GetId(&defaultDeviceId);
        if (hr != S_OK) {
            safeCleanup();
            THROW_ERROR("Failed to get default audio device ID: " << hr);
        }
    }

    hr = audioEnum->EnumAudioEndpoints(
        dataflow,
        DEVICE_STATE_ACTIVE,
        &collection
    );

    if (hr != S_OK) {
        safeCleanup();
        THROW_ERROR("Failed to enum audio endpoints: " << hr);
    }

    unsigned int deviceCount = 0;
    hr = collection->GetCount(&deviceCount);
    if (hr != S_OK) {
        safeCleanup();
        THROW_ERROR("Failed to get audio device count: " << hr);
    }

    for (auto i = 0; i < deviceCount; ++i) {
        hr = collection->Item(i, &device);
        if (hr != S_OK) {
            safeCleanup();
            LOG_WARNING("Failed to get audio device: " << hr);
        }

        hr = device->GetId(&deviceId);
        if (hr != S_OK) {
            safeCleanup();
            THROW_ERROR("Failed to get audio device ID: " << hr);
        }

        std::string sDeviceName = getDeviceName(device);
        const auto ourDevice = service::recorder::audio::SingleAudioDevice(sDeviceName, shared::strings::wcsToUtf8(deviceId));
        resp.options.push_back(ourDevice);

        bool isDefault = false;
        if (defaultDeviceId && lstrcmpW(defaultDeviceId, deviceId) == 0) {
            isDefault = true;
            resp.default = ourDevice;
        }

        LOG_INFO("FOUND AUDIO DEVICE: " << sDeviceName << " " << isDefault << std::endl);

        CoTaskMemFree(deviceId);
        deviceId = nullptr;

        device->Release();
        device = nullptr;
    }

    safeCleanup();
    return resp;
}

std::string WASAPIInterface::getDeviceName(IMMDevice* device) {
    CComPtr<IPropertyStore> props = nullptr;
    HRESULT hr = device->OpenPropertyStore(STGM_READ, &props);
    if (hr != S_OK) {
        THROW_ERROR("Failed to get audio device props: " << hr);
        return "";
    }

    PROPVARIANT value;
    PropVariantInit(&value);

    hr = props->GetValue(PKEY_Device_FriendlyName, &value);
    if (hr != S_OK) {
        THROW_ERROR("Failed to get audio device friendly name: " << hr);
        return "";
    }

    std::wstring wDeviceName(value.pwszVal);
    return shared::strings::wcsToUtf8(wDeviceName);
}

CComPtr<IMMDevice> WASAPIInterface::getDeviceFromName(service::recorder::audio::EAudioDeviceDirection dir, const std::string& name) {
    // When getting a device by name we have to pretty much go through the list of devices and find the one with a name that matches. F U N.
    try {
        const auto choices = getDeviceListing(dir);
        for (const auto& d: choices.options) {
            if (d.name == name) {
                return getDeviceFromId(d.id);
            }
        }
    } catch (std::exception& ex) {
        LOG_WARNING("Failed to get device listing: " << ex.what() << std::endl);
    }

    return nullptr;
}

CComPtr<IMMDevice> WASAPIInterface::getDeviceFromId(const std::string& id) {
    CComPtr<IMMDeviceEnumerator> audioEnum;
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(IMMDeviceEnumerator),
        (LPVOID*)&audioEnum
    );

    if (hr != S_OK) {
        LOG_WARNING("Failed to create IMMDeviceEnumerator: " << hr);
        return nullptr;
    }

    const auto wId = shared::strings::utf8ToWcs(id);

    CComPtr<IMMDevice> device;
    hr = audioEnum->GetDevice(wId.c_str(), &device);
    if (hr != S_OK) {
        LOG_WARNING("Failed to get device from ID: " << hr);
        return nullptr;
    }

    return device;
}

CComPtr<IMMDevice> WASAPIInterface::getDefaultDevice(service::recorder::audio::EAudioDeviceDirection dir) {
    CComPtr<IMMDeviceEnumerator> audioEnum;
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(IMMDeviceEnumerator),
        (LPVOID*)&audioEnum
    );

    if (hr != S_OK) {
        LOG_WARNING("Failed to create IMMDeviceEnumerator: " << hr);
        return nullptr;
    }

    CComPtr<IMMDevice> device;
    hr = audioEnum->GetDefaultAudioEndpoint((dir == service::recorder::audio::EAudioDeviceDirection::Output) ? eRender : eCapture, eConsole, &device);
    if (hr != S_OK) {
        LOG_WARNING("Failed to get default device: " << hr);
        return nullptr;
    }
    return device;
}

}
#endif