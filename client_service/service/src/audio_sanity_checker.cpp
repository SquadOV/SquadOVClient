#include "shared/log/log.h"
#include "shared/time/ntp_client.h"
#include "system/settings.h"
#include "shared/strings/strings.h"
#include "shared/filesystem/common_paths.h"
#include "recorder/audio/win32/wasapi_interface.h"
#include "hardware/hardware.h"
#include <fstream>
#include <unordered_set>

namespace fs = std::filesystem;

void sanitize(service::system::AudioDeviceSettings& device, service::recorder::audio::EAudioDeviceDirection dir) {
    LOG_INFO("Sanitizing audio device...[Name: " << device.device << "\tId: " << device.id << "]" << std::endl);

    // If the ID already exists we don't need to do anything.
    if (!device.id.empty()) {
        LOG_INFO("...Id already exists - skipping." << std::endl);
        return;
    }

    if (device.device.rfind("Default Device") == 0) {
        LOG_INFO("...Default device found." << std::endl);
        return;
    }

    // Try to find the audio device given its name.
    // If it does exist, fill in the ID. If it doesn't exist then whatever. Yolo let the user live their life ya feel.
    auto audioDevice = service::recorder::audio::win32::WASAPIInterface::getDeviceFromName(dir, device.device);
    if (!audioDevice) {
        LOG_WARNING("...Device not found." << std::endl);
    }

    LPWSTR deviceId = nullptr;
    HRESULT hr = audioDevice->GetId(&deviceId);
    if (hr != S_OK) {
        LOG_WARNING("...Failed to get ID: " << hr << std::endl);
        return;
    }

    device.id = shared::strings::wcsToUtf8(deviceId);

    CoTaskMemFree(deviceId);
    deviceId = nullptr;
    LOG_INFO("\tFinished. [Name: " << device.device << "\tId: " << device.id << "]" << std::endl);
}

int main(int argc, char** argv) {
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    shared::log::Log::initializeGlobalLogger("audio_sanity.log");

    // NTP can't be init before the logger since we log stuff inside the NTP client.
    shared::time::NTPClient::singleton()->enable(true);
    shared::time::NTPClient::singleton()->initialize();

    LOG_INFO("Starting Audio Sanity Checker..." << std::endl);
    service::system::getCurrentSettings()->reloadSettingsFromFile();

    auto recordingSettings = service::system::getCurrentSettings()->recording();
    LOG_INFO("Sanitize output devices..." << std::endl);
    for (auto& device: recordingSettings.outputDevices) {
        sanitize(device, service::recorder::audio::EAudioDeviceDirection::Output);
    }

    LOG_INFO("Sanitize input devices..." << std::endl);
    for (auto& device: recordingSettings.inputDevices) {
        sanitize(device, service::recorder::audio::EAudioDeviceDirection::Input);
    }

    LOG_INFO("Checking per-process recording..." << std::endl);
    const auto sysHw = service::hardware::getSystemHardware();
    try {
        const auto majorInt = std::stoi(sysHw.os.majorVersion);
        const std::unordered_set<std::string> windows10MinorVersionSupport = { "21H1", "21H2" };

        recordingSettings.perProcessRecordingOsCheck = (sysHw.os.name == "Windows")
            && (
                ((majorInt == 10) && windows10MinorVersionSupport.find(sysHw.os.minorVersion) != windows10MinorVersionSupport.end()) ||
                (majorInt == 11)
            );
                
    } catch (std::exception& ex) {
        LOG_INFO("...Failed to check OS: " << sysHw << "\t" << ex.what() << std::endl);
        recordingSettings.perProcessRecordingOsCheck = false;
    }

    LOG_INFO("Saving changes to disk..." << std::endl);
    auto rawData = service::system::getCurrentSettings()->raw();

    rawData["record"]["outputDevices"] = shared::json::JsonConverter<std::vector<service::system::AudioDeviceSettings>>::to(recordingSettings.outputDevices);
    rawData["record"]["inputDevices"] = shared::json::JsonConverter<std::vector<service::system::AudioDeviceSettings>>::to(recordingSettings.inputDevices);
    rawData["record"]["perProcessRecordingOsCheck"] = shared::json::JsonConverter<bool>::to(recordingSettings.perProcessRecordingOsCheck);

    const auto fpath = shared::filesystem::getSquadOvUserSettingsFile();
    const auto tmpPath = fpath.parent_path() / fs::path("settings.json.tmp");

    std::ofstream ofs(tmpPath);
    ofs << std::setw(4) << rawData;
    ofs.close();

    fs::rename(tmpPath, fpath);
    return 0;
}