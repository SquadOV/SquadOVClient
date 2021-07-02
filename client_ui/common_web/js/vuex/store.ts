import Vue from 'vue'
import { StoreOptions } from 'vuex'
import { SquadOVUser } from '@client/js/squadov/user'
import { SquadOvLocalSettings, loadLocalSettings, saveLocalSettings, SquadOvOverlay, AudioDeviceSettings} from '@client/js/system/settings'
import { createDefaultState } from '@client/js/system/state'
import { FeatureFlags } from '@client/js/squadov/features'
import { apiClient, ApiData } from '@client/js/api'
import { RootState } from '@client/js/vuex/state'
import { TrackedUserStatus } from '@client/js/squadov/status'
import { SquadOvGames } from '@client/js/squadov/game'

export const RootStoreOptions : StoreOptions<RootState> = {
    strict: true,
    state: {
        currentUser: null,
        hasValidSession: true,
        features: {
            maxRecordPixelY: 0,
            maxRecordFps: 0,
            allowRecordUpload: false,
            allowWowCombatLogUpload: false,
        },
/// #if DESKTOP
        settings: null,
        currentState: createDefaultState(),
/// #endif
        status: { status:{} },
    },
    mutations: {
        setUser(state : RootState, user : SquadOVUser) {
            state.currentUser = user
        },
        setSettings(state: RootState, settings: SquadOvLocalSettings) {
/// #if DESKTOP
            state.settings = settings
/// #endif
        },
        changeRecordSettingRes(state: RootState, res: number) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.resY = res
            saveLocalSettings(state.settings)
/// #endif
        },
        changeRecordSettingFps(state: RootState, fps: number) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.fps = fps
            saveLocalSettings(state.settings)
/// #endif
        },
        changeUseVideoHw(state: RootState, b: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.useVideoHw = b
            saveLocalSettings(state.settings)
/// #endif
        },
        changeUseHwEncoder(state: RootState, b: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.useHwEncoder = b
            saveLocalSettings(state.settings)
/// #endif
        },
        changeUseVfr32(state: RootState, b: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.useVfr3 = b
            saveLocalSettings(state.settings)
/// #endif
        },
        changeLocalRecording(state: RootState, params: {use: boolean, loc: string, limit: number}) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            
            state.settings.record.useLocalRecording = params.use
            state.settings.record.localRecordingLocation = params.loc
            state.settings.record.maxLocalRecordingSizeGb = params.limit
            saveLocalSettings(state.settings)
/// #endif
        },
        changeOutputDevice(state: RootState, params: AudioDeviceSettings[]) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.outputDevices = JSON.parse(JSON.stringify(params))
            saveLocalSettings(state.settings)
/// #endif
        },
        changeInputDevice(state: RootState, params: AudioDeviceSettings[]) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.inputDevices = JSON.parse(JSON.stringify(params))
            saveLocalSettings(state.settings)
/// #endif
        },
        toggleRecordingPause(state: RootState) {
/// #if DESKTOP
            state.currentState.paused = !state.currentState.paused
/// #endif
        },
        resetState(state: RootState) {
/// #if DESKTOP
            state.currentState = createDefaultState()
/// #endif
        },
        setRunningGames(state: RootState, games: string[]) {
/// #if DESKTOP
            state.currentState.runningGames = games
/// #endif
        },
        setRecordingGames(state: RootState, games: string[]) {
/// #if DESKTOP
            state.currentState.recordingGames = games
/// #endif
        },
        setRunOnStartup(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.runOnStartup = v
            saveLocalSettings(state.settings)
/// #endif
        },
        setMinimizeToSystemTray(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.minimizeToTray = v
            saveLocalSettings(state.settings)
/// #endif
        },
        setMinimizeOnClose(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.minimizeOnClose = v
            saveLocalSettings(state.settings)
/// #endif
        },
        setSetupWizardRun(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.setupWizardRun = v
            saveLocalSettings(state.settings)
/// #endif
        },
        setMaxUploadSpeed(state: RootState, v: number | null) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.maxUploadSpeed = v
            saveLocalSettings(state.settings)
/// #endif
        },
        markValidSession(state: RootState, v: boolean) {
            state.hasValidSession = v
        },
        setFeatureFlags(state: RootState, flags: FeatureFlags) {
            state.features = flags
        },
        setUserActivityStatus(state: RootState, params: {userId: number, status: TrackedUserStatus}) {
            Vue.set(state.status.status, params.userId, params.status)
        },
        changeRecordingBitrate(state: RootState, params: {enable: boolean, bitrateKbps: number}) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.useBitrate = params.enable
            state.settings.record.bitrateKbps = params.bitrateKbps
            saveLocalSettings(state.settings)
/// #endif
        },
        changeVodEndDelaySeconds(state: RootState, v: number) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.vodEndDelaySeconds = v
            saveLocalSettings(state.settings)
/// #endif
        },
        changePushToTalk(state: RootState, params: {enable: boolean, ptt: number[]}) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.usePushToTalk = params.enable
            state.settings.keybinds.pushToTalk = params.ptt
            saveLocalSettings(state.settings)
/// #endif
        },
        setEnableNtp(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.enableNtp = v
            saveLocalSettings(state.settings)
/// #endif
        },
        changeSmallStep(state: RootState, v: number) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.playback.smallStepSize = v
            saveLocalSettings(state.settings)
/// #endif
        },
        changeLargeStep(state: RootState, v: number) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.playback.largeStepSize = v
            saveLocalSettings(state.settings)
/// #endif
        },
        setAnonymousAnalytics(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.anonymousAnalytics = v
            saveLocalSettings(state.settings)
/// #endif
        },
        changeDisabledGames(state: RootState, games: SquadOvGames[]) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.disabledGames = games
            saveLocalSettings(state.settings)
/// #endif
        },
        changeOverlayEnabled(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.overlays.enabled = v
            saveLocalSettings(state.settings)
/// #endif
        },
        changeOverlayLayers(state: RootState, v: SquadOvOverlay[]) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.overlays.layers = JSON.parse(JSON.stringify(v))
            saveLocalSettings(state.settings, true)
/// #endif
        },
        changeAudioDriftCompensation(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.useAudioDriftCompensation = v
            saveLocalSettings(state.settings)
/// #endif
        },
    },
    actions: {
        async reloadLocalSettings(context) {
/// #if DESKTOP            
            let settings = await loadLocalSettings()
            context.commit('setSettings', settings)
/// #endif
        },
        async loadUserFeatureFlags(context) {
            if (!context.state.currentUser) {
                return
            }
            apiClient.getFeatureFlags(context.state.currentUser.id).then((resp: ApiData<FeatureFlags>) => {
                context.commit('setFeatureFlags', resp.data)
            }).catch((err: any) => {
                console.log('Failed to load feature flags: ', err)
            })
        }
    }
}