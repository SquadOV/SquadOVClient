import { StoreOptions } from 'vuex'
import { SquadOVUser } from '@client/js/squadov/user'
import { SquadOvLocalSettings, loadLocalSettings, saveLocalSettings} from '@client/js/system/settings'
import { SquadOvState, createDefaultState } from '@client/js/system/state' 
import { Root } from 'protobufjs'

interface RootState {
    currentUser: SquadOVUser | null
    settings: SquadOvLocalSettings | null
    currentState: SquadOvState
}

export const RootStoreOptions : StoreOptions<RootState> = {
    strict: true,
    state: {
        currentUser: null,
        settings: null,
        currentState: createDefaultState()
    },
    mutations: {
        setUser(state : RootState, user : SquadOVUser) {
            state.currentUser = user
        },
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
        }
    },
    actions: {
        async reloadLocalSettings(context) {
            let settings = await loadLocalSettings()
            context.commit('setSettings', settings)
        }
    }
}