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
    IPropertyStore* props = nullptr;

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

        if (props) {
            props->Release();
            props = nullptr;
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
        (dir == service::recorder::audio::EAudioDeviceDirection::Output) ? eConsole : eCommunications,
        &defaultDevice
    );

    if (hr != S_OK) {
        safeCleanup();
        THROW_ERROR("Failed to get default audio endpoint: " << hr);
    }

    hr = defaultDevice->GetId(&defaultDeviceId);
    if (hr != S_OK) {
        safeCleanup();
        THROW_ERROR("Failed to get default audio device ID: " << hr);
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

        hr = device->OpenPropertyStore(STGM_READ, &props);
        if (hr != S_OK) {
            safeCleanup();
            THROW_ERROR("Failed to get audio device props: " << hr);
        }

        PROPVARIANT value;
        PropVariantInit(&value);

        hr = props->GetValue(PKEY_Device_FriendlyName, &value);
        if (hr != S_OK) {
            safeCleanup();
            THROW_ERROR("Failed to get audio device friendly name: " << hr);
        }

        std::wstring wDeviceName(value.pwszVal);
        std::string sDeviceName = shared::strings::wcsToUtf8(wDeviceName);
        resp.options.push_back(sDeviceName);

        if (lstrcmpW(defaultDeviceId, deviceId) == 0) {
            resp.default = sDeviceName;
        }

        CoTaskMemFree(deviceId);
        deviceId = nullptr;
        
        device->Release();
        device = nullptr;
    }

    safeCleanup();
    return resp;
}

}

#endif