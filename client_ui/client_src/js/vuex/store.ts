import { StoreOptions } from 'vuex'
import { SquadOVUser } from '@client/js/squadov/user'
import { SquadOvLocalSettings, loadLocalSettings, saveLocalSettings} from '@client/js/system/settings'

interface RootState {
    currentUser: SquadOVUser | null
    settings: SquadOvLocalSettings | null
}

export const RootStoreOptions : StoreOptions<RootState> = {
    strict: true,
    state: {
        currentUser: null,
        settings: null
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
    },
    actions: {
        async reloadLocalSettings(context) {
            let settings = await loadLocalSettings()
            context.commit('setSettings', settings)
        }
    }
}