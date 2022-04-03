import Vue from 'vue'
import { StoreOptions } from 'vuex'
import { SquadOVUser } from '@client/js/squadov/user'
import { SquadOvLocalSettings, loadLocalSettings, saveLocalSettings, SquadOvOverlay, AudioDeviceSettings, ProcessAudioRecordSettings, WowDisabledInstance} from '@client/js/system/settings'
import { createDefaultState } from '@client/js/system/state'
import { FeatureFlags } from '@client/js/squadov/features'
import { apiClient, ApiData } from '@client/js/api'
import { RootState } from '@client/js/vuex/state'
import { TrackedUserStatus } from '@client/js/squadov/status'
import { SquadOvGames } from '@client/js/squadov/game'
import { computeFileFolderSizeGb } from '@client/js/system/settings'

export const RootStoreOptions : StoreOptions<RootState> = {
    // Absolutely do not use strict here. We do a lot of mutations (particularly for user state)
    // and it slows things down BY A LOT.
    strict: false,
    state: {
        currentUser: null,
        attemptUserLoad: false,
        redirectUrl: null,
        hasValidSession: true,
        features: {
            maxRecordPixelY: 0,
            maxRecordFps: 0,
            allowRecordUpload: false,
            allowWowCombatLogUpload: false,
            enableUserProfiles: false,
            maxBitrateKbps: 0,
        },
/// #if DESKTOP
        settings: null,
        currentState: createDefaultState(),
        localDiskSpaceRecordUsageGb: null,
/// #endif
        status: { status:{} },
        hasUpdate: false,
        serviceError: false,
        muteInviteFriendsPopUp: !!window.localStorage.getItem('muteInviteFriendsPopUp'),
        successfullyVisitedVideo: false,
        displayInviteFriendPopUp: false,
        forceHideNav: false,
        cachedIp: null,
        requiresRestart: false,
    },
    mutations: {
        attemptUserLoad(state: RootState, b: boolean) {
            state.attemptUserLoad = b
        },
        displayInviteFriendPopUp(state: RootState, b: boolean) {
            state.displayInviteFriendPopUp = b
        },
        successfullyVisitedVideo(state: RootState, b: boolean) {
            state.successfullyVisitedVideo = b
        },
        muteInviteFriendsPopUp(state: RootState) {
            window.localStorage.setItem('muteInviteFriendsPopUp', 'true')
            state.muteInviteFriendsPopUp = true
        },
        notifyUpdate(state: RootState) {
            state.hasUpdate = true
        },
        notifyServiceError(state: RootState) {
            state.serviceError = true
        },
        setUser(state : RootState, user : SquadOVUser) {
            state.currentUser = user
        },
        setRedirectUrl(state: RootState, url: string) {
            state.redirectUrl = url
        },
        setSettings(state: RootState, settings: SquadOvLocalSettings) {
/// #if DESKTOP
            state.settings = settings
/// #endif
        },
        changeRecordSettingQuality(state: RootState, res: number) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.bitrateKbps = res
            saveLocalSettings(state.settings)
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
            state.settings.record.useVideoHw2 = b
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
        changeUseVfr(state: RootState, b: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.useVfr4 = b
            saveLocalSettings(state.settings)
/// #endif
        },
        changeuseWGC2(state: RootState, b: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.useWGC2 = b
            saveLocalSettings(state.settings)
/// #endif
        },
        changeRecordMouse(state: RootState, b: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.recordMouse2 = b
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
        changeUseWASAPIRecording(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.useWASAPIRecording2 = v
            saveLocalSettings(state.settings)
/// #endif
        },
        changePerProcessRecordingOsCheck(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.perProcessRecordingOsCheck = v
            saveLocalSettings(state.settings)
/// #endif
        },
        changeUseProcessAudioRecording(state: RootState, params: {enable: boolean, game: boolean, processes: ProcessAudioRecordSettings[]}) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.usePerProcessRecording = params.enable
            state.settings.record.recordGameAudio = params.game
            state.settings.record.processesToRecord = params.processes
            saveLocalSettings(state.settings)
/// #endif
        },
        changeGameAudioVolume(state: RootState, v: number) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.gameAudioVolume = v
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
        markValidSession(state: RootState, v: boolean) {
            state.hasValidSession = v
        },
        setFeatureFlags(state: RootState, flags: FeatureFlags) {
            state.features = flags
        },
        bulkUpdateUserActivityStatus(state: RootState, params: { [userId: number] : TrackedUserStatus }) {
            for (let [userId, st] of Object.entries(params)) {
                state.status.status[parseInt(userId)] = st
            }

            //@ts-ignore
            state.status.status.__ob__.dep.notify()
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
        changePushToTalk(state: RootState, params: {enable: boolean, ptt: number[], altPtt: number[]}) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.usePushToTalk = params.enable
            state.settings.keybinds.pushToTalk = [...params.ptt]
            state.settings.keybinds.pushToTalk2 = [...params.altPtt]
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

            window.localStorage.setItem('muteDisabledGamesAlert', 'false')
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
        changeWowLogTimeout(state: RootState, params: {use: boolean, seconds: number}) {
/// #if DESKTOP
            if (!state.settings || params.seconds === null) {
                return
            }
            state.settings.games.wow.useCombatLogTimeout = params.use
            state.settings.games.wow.timeoutSeconds2 = params.seconds
            saveLocalSettings(state.settings)
/// #endif  
        },
        changeVoiceBasicNoiseThreshold(state: RootState, params: {use: boolean, threshold: number}) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.useVoiceBasicNoiseFilter = params.use
            state.settings.record.voiceFilterThresholdDb = params.threshold
            saveLocalSettings(state.settings)
/// #endif  
        },
        changeVoiceNoiseSuppression(state: RootState, use: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.useVoiceSpeechNoiseReduction = use
            saveLocalSettings(state.settings)
/// #endif        
        },
        changeWowMinimumTimeSecondsToRecord(state: RootState, v: number) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.games.wow.minimumTimeSecondsToRecord = v
            saveLocalSettings(state.settings)
/// #endif
        },
        changeWowRecordArenas(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.games.wow.recordArenas = v
            saveLocalSettings(state.settings)
/// #endif
        },
        changeWowRecordDungeons(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.games.wow.recordDungeons = v
            saveLocalSettings(state.settings)
/// #endif
        },
        changeWowRecordBattlegrounds(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.games.wow.recordBattlegrounds = v
            saveLocalSettings(state.settings)
/// #endif
        },
        changeWowRecordKeystones(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.games.wow.recordKeystones = v
            saveLocalSettings(state.settings)
/// #endif
        },
        changeWowRecordEncounters(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.games.wow.recordEncounters = v
            saveLocalSettings(state.settings)
/// #endif
        },
        changeWowRecordScenarios(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.games.wow.recordScenarios = v
            saveLocalSettings(state.settings)
/// #endif
        },
        changeWowDoNotRecordInstances(state: RootState, v: WowDisabledInstance[]) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.games.wow.doNotRecordInstances = v
            saveLocalSettings(state.settings)
/// #endif
        },
        changeValorantRecordingSettings(state: RootState, params: {standard?: boolean, spikeRush?: boolean, deathmatch?: boolean, other?: boolean }) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }

            if (params.standard !== undefined) {
                state.settings.games.valorant.recordStandard = params.standard
            }

            if (params.spikeRush !== undefined) {
                state.settings.games.valorant.recordSpikeRush = params.spikeRush
            }

            if (params.deathmatch !== undefined) {
                state.settings.games.valorant.recordDeathmatch = params.deathmatch
            }

            if (params.other !== undefined) {
                state.settings.games.valorant.recordOther = params.other
            }
            saveLocalSettings(state.settings)
/// #endif  
        },
        changeNeedConfirmManualStop(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.needConfirmManualStop = v
            saveLocalSettings(state.settings)
/// #endif
        },
        updateLocalStorageUsage(state: RootState, val: number) {
            state.localDiskSpaceRecordUsageGb = val
        },
        changeForceHideNav(state: RootState, v: boolean) {
            state.forceHideNav = v
        },
        updateCachedIp(state: RootState, ip: string) {
            state.cachedIp = ip
        },
        changeDisablePostGamePopup(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.disablePostGamePopup = v
            saveLocalSettings(state.settings)
/// #endif
        },
        changeHidePostGamePopupUntil(state: RootState, v: Date | null) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.hidePostGamePopupUntil = v
            saveLocalSettings(state.settings)
/// #endif
        },
        changeUseHwAccel(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.requiresRestart = true
            state.settings.useHwAccel = v
            saveLocalSettings(state.settings)
/// #endif            
        },
        changeWowShowAddonWarnings(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.games.wow.showAddonWarnings = v
            saveLocalSettings(state.settings)
/// #endif            
        },
        changeUseCbr(state: RootState, v: boolean) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.useCbr = v
            saveLocalSettings(state.settings)
/// #endif            
        },
        changeBookmarkKeybind(state: RootState, v: number[]) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.keybinds.bookmark = [...v]
            saveLocalSettings(state.settings)
/// #endif         
        },
        changeClipKeybind(state: RootState, v: number[]) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.keybinds.clip2 = [...v]
            saveLocalSettings(state.settings)
/// #endif         
        },
        changeInstantClipLength(state: RootState, v: number) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.instantClipLengthSeconds = v
            saveLocalSettings(state.settings)
/// #endif         
        }
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
                console.error('Failed to load feature flags: ', err)
            })
        },
        recomputeLocalDiskSpaceRecordUsage(context) {
            if (!context.state.settings?.record.localRecordingLocation) {
                return
            }

            computeFileFolderSizeGb(context.state.settings.record.localRecordingLocation)
                .then((resp: number) => {
                    context.commit('updateLocalStorageUsage', resp)
                })
                .catch((err: any) => {
                    console.error('Failed to get local disk space record usage: ', err)
                })
        }
    }
}