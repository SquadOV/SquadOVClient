import Vue from 'vue'
import { StoreOptions } from 'vuex'
import { SquadOVUser } from '@client/js/squadov/user'
import { SquadOvLocalSettings, loadLocalSettings, saveLocalSettings} from '@client/js/system/settings'
import { createDefaultState } from '@client/js/system/state'
import { FeatureFlags } from '@client/js/squadov/features'
import { apiClient, ApiData } from '@client/js/api'
import { RootState } from '@client/js/vuex/state'
import { TrackedUserStatus } from '@client/js/squadov/status'

export const RootStoreOptions : StoreOptions<RootState> = {
    strict: true,
    state: {
        currentUser: null,
        hasValidSession: true,
        features: {
            enableLol: false,
            enableTft: false,
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
        changeOutputDevice(state: RootState, params: {device: string, volume : number}) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.outputDevice = params.device
            state.settings.record.outputVolume = params.volume
            saveLocalSettings(state.settings)
/// #endif
        },
        changeInputDevice(state: RootState, params: {device: string, volume : number}) {
/// #if DESKTOP
            if (!state.settings) {
                return
            }
            state.settings.record.inputDevice = params.device
            state.settings.record.inputVolume = params.volume
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
        markValidSession(state: RootState, v: boolean) {
            state.hasValidSession = v
        },
        setFeatureFlags(state: RootState, flags: FeatureFlags) {
            state.features = flags
        },
        setUserActivityStatus(state: RootState, params: {userId: number, status: TrackedUserStatus}) {
            Vue.set(state.status.status, params.userId, params.status)
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
                console.log('Failed to load feature flags: ', err)
            })
        }
    }
}