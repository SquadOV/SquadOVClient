import { StoreOptions } from 'vuex'
import { SquadOVUser } from '@client/js/squadov/user'
/// #if DESKTOP
import { SquadOvLocalSettings, loadLocalSettings, saveLocalSettings} from '@client/js/system/settings'
import { SquadOvState, createDefaultState } from '@client/js/system/state' 
/// #endif

interface RootState {
    currentUser: SquadOVUser | null
    hasValidSession: boolean
/// #if DESKTOP
    settings: SquadOvLocalSettings | null
    currentState: SquadOvState
/// #endif
}

export const RootStoreOptions : StoreOptions<RootState> = {
    strict: true,
    state: {
        currentUser: null,
        hasValidSession: true,
/// #if DESKTOP
        settings: null,
        currentState: createDefaultState()
/// #endif
    },
    mutations: {
        setUser(state : RootState, user : SquadOVUser) {
            state.currentUser = user
        },
/// #if DESKTOP
        setSettings(state: RootState, settings: SquadOvLocalSettings) {
            state.settings = settings
        },
        changeRecordSettingRes(state: RootState, res: number) {
            if (!state.settings) {
                return
            }
            state.settings.record.resY = res
            saveLocalSettings(state.settings)
        },
        changeRecordSettingFps(state: RootState, fps: number) {
            if (!state.settings) {
                return
            }
            state.settings.record.fps = fps
            saveLocalSettings(state.settings)
        },
        toggleRecordingPause(state: RootState) {
            state.currentState.paused = !state.currentState.paused
        },
        resetState(state: RootState) {
            state.currentState = createDefaultState()
        },
        setRunningGames(state: RootState, games: string[]) {
            state.currentState.runningGames = games
        },
        setRecordingGames(state: RootState, games: string[]) {
            state.currentState.recordingGames = games
        },
/// #endif
        markValidSession(state: RootState, v: boolean) {
            state.hasValidSession = v
        },
    },
    actions: {
/// #if DESKTOP
        async reloadLocalSettings(context) {
            let settings = await loadLocalSettings()
            context.commit('setSettings', settings)
        }
/// #endif
    }
}