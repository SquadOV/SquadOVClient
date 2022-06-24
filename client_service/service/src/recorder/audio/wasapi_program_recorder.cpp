#include "recorder/audio/wasapi_program_recorder.h"
#include "recorder/audio/win32/wasapi_interface.h"
#include "shared/audio/timing.h"
#include "shared/strings/strings.h"
#include "shared/log/log.h"
#include "shared/errors/error.h"
#include "shared/system/win32/interfaces/win32_system_process_interface.h"
#include "shared/strings/strings.h"
#include "process_watcher/process/process.h"
#include "recorder/audio/audio_packet_view.h"
#include "recorder/audio/fixed_size_audio_packet.h"
#include "recorder/audio/wasapi_audio_client_recorder.h"

#include <audioclientactivationparams.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <wrl/implements.h>
#include <wil/com.h>
#include <wil/result.h>

using namespace Microsoft::WRL;

namespace service::recorder::audio {

class CompletionHandler: public RuntimeClass<RuntimeClassFlags<ClassicCom>, FtmBase, IActivateAudioInterfaceCompletionHandler> {
public:
    CompletionHandler();

    void wait();
    bool success() const { return _success; }
    wil::com_ptr<IAudioClient> client() const { return _audioClient; }

    // Called by windows to notify us that recording was successfully activated.
    STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation* activateOperation);

private:
    wil::unique_event_nothrow _completeEvent;
    wil::com_ptr<IAudioClient> _audioClient;
    bool _success = false;
};

class WasapiProgramRecorderImpl {
public:
    WasapiProgramRecorderImpl(OSPID pid, double volume);
    ~WasapiProgramRecorderImpl();

    void startRecording();
    void setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex);
    void stop();
    void loadDevice(EAudioDeviceDirection dir, const service::system::AudioDeviceSettings& device, AudioDeviceSet& deviceSet);
    double initialVolume() const { return _initialVolume; }
    void setVolume(double volume) { _internal->setVolume(volume); }

    bool exists() const { return _process.has_value() && _internal && _internal->exists(); }
    const AudioPacketProperties& props() const { return _internal->props(); }
    const std::string& deviceName() const { return _deviceName; }

private:
    void printWarning(const std::string& msg, HRESULT hr) const;

    OSPID _pid;
    std::optional<process_watcher::process::Process> _process;
    double _initialVolume = 1.0;
    WasapiAudioClientRecorderPtr _internal;
    std::string _deviceName;
};

WasapiProgramRecorderImpl::WasapiProgramRecorderImpl(OSPID pid, double volume):
    _pid(pid),
    _initialVolume(volume)
{
    // Find the process again - mainly just so we some additional information about what process we're trying to record for debugging.
    auto itf = std::make_shared<shared::system::win32::interfaces::Win32SystemProcessInterface>();
    process_watcher::process::ProcessRunningState processState(itf);
    processState.update();

    _process = processState.getProcesssRunningByPid(_pid, false);
    if (_process) {
        LOG_INFO("Trying to record audio from process: " << _process.value().name() << std::endl);
    } else {
        LOG_WARNING("...No process detected for audio recording for PID: " << _pid << std::endl);
        return;
    }
    _deviceName =  shared::strings::wcsToUtf8(_process->name());

    // Need to do all this on construction because the internal object being constructed is a major part of success
    // and is needed to determine whether or not we can actually record.
    AUDIOCLIENT_ACTIVATION_PARAMS audioclientActivationParams = {};
    audioclientActivationParams.ActivationType = AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK;
    audioclientActivationParams.ProcessLoopbackParams.ProcessLoopbackMode = PROCESS_LOOPBACK_MODE_INCLUDE_TARGET_PROCESS_TREE;
    audioclientActivationParams.ProcessLoopbackParams.TargetProcessId = _pid;

    PROPVARIANT activateParams = {};
    activateParams.vt = VT_BLOB;
    activateParams.blob.cbSize = sizeof(audioclientActivationParams);
    activateParams.blob.pBlobData = (BYTE*)&audioclientActivationParams;

    // Windows does some async magic here so the result of the activation
    // process will be called on the CompletionHandler struct so we need to
    // wait for that to be triggered before continuing. We want to do it this
    // way so we can do the standard thing of starting the recording threads in
    // the startRecording function - seems reasonable.
    wil::com_ptr_nothrow<IActivateAudioInterfaceAsyncOperation> asyncOp;
    CompletionHandler completion;
    HRESULT hr = ActivateAudioInterfaceAsync(
        VIRTUAL_AUDIO_DEVICE_PROCESS_LOOPBACK,
        __uuidof(IAudioClient),
        &activateParams,
        &completion,
        &asyncOp
    );

    if (hr != S_OK) {
        LOG_ERROR("Failed to start activate audio interface." << std::endl);
        return;
    }

    completion.wait();
    if (!completion.success()) {
        LOG_ERROR("Failed to complete audio interface activation." << std::endl);
        return;
    }

    LOG_INFO("...Successfully activated WASAPI process loopback..." << std::endl);
    // At this point we have an audio client so we should be able to do the normal WASAPI recording.
    _internal = std::make_unique<WasapiAudioClientRecorder>(
        completion.client(),
        shared::strings::wcsToUtf8(_process.value().name()),
        false,
        true
    );
    _internal->setVolume(_initialVolume);
}

WasapiProgramRecorderImpl::~WasapiProgramRecorderImpl() {
}

void WasapiProgramRecorderImpl::loadDevice(EAudioDeviceDirection dir, const service::system::AudioDeviceSettings& device, AudioDeviceSet& deviceSet) {
    THROW_ERROR("Unsupported.");
}

CompletionHandler::CompletionHandler() {
    _completeEvent.create(wil::EventOptions::None);
}

void CompletionHandler::wait() {
    _completeEvent.wait();
}

HRESULT CompletionHandler::ActivateCompleted(IActivateAudioInterfaceAsyncOperation* operation) {
    auto setOnExit = _completeEvent.SetEvent_scope_exit();

    HRESULT hrActivateResult = E_UNEXPECTED;
    wil::com_ptr_nothrow<IUnknown> punkAudioInterface;
    HRESULT hr = operation->GetActivateResult(&hrActivateResult, &punkAudioInterface);
    if (hr != S_OK) {
        LOG_WARNING("Failed to get activate result [1]: " << hr << std::endl);
        return hr;
    }

    if (hrActivateResult != S_OK) {
        LOG_WARNING("Failed to get activate result [2]: " << hrActivateResult << std::endl);
        return hr;
    }

    hr = punkAudioInterface.copy_to(&_audioClient);
    if (hr != S_OK) {
        LOG_WARNING("Failed to audio client " << hr << std::endl);
        return hr;
    }

    _success = true;
    return S_OK;
}

void WasapiProgramRecorderImpl::startRecording() {
    _internal->startRecording();
}

void WasapiProgramRecorderImpl::setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex) {
    _internal->setActiveEncoder(encoder, encoderIndex);
}

void WasapiProgramRecorderImpl::stop() {
    _internal->stop();
}

WasapiProgramRecorder::WasapiProgramRecorder(OSPID pid, double volume):
    _impl(new WasapiProgramRecorderImpl(pid, volume))
{
}

WasapiProgramRecorder::~WasapiProgramRecorder() {

}

void WasapiProgramRecorder::startRecording() {
    _impl->startRecording();
}

void WasapiProgramRecorder::setActiveEncoder(service::recorder::encoder::AvEncoder* encoder, size_t encoderIndex) {
    _impl->setActiveEncoder(encoder, encoderIndex);
}

void WasapiProgramRecorder::stop() {
    _impl->stop();
}

void WasapiProgramRecorder::loadDevice(EAudioDeviceDirection dir, const service::system::AudioDeviceSettings& device, AudioDeviceSet& deviceSet) {
    _impl->loadDevice(dir, device, deviceSet);
}

double WasapiProgramRecorder::initialVolume() const {
    return _impl->initialVolume();
}

void WasapiProgramRecorder::setVolume(double volume) {
    _impl->setVolume(volume);
}

bool WasapiProgramRecorder::exists() const {
    return _impl->exists();
}

const std::string& WasapiProgramRecorder::deviceName() const {
    return _impl->deviceName();
}

const AudioPacketProperties& WasapiProgramRecorder::props() const {
    return _impl->props();
}

}