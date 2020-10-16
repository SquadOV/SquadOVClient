import { StoreOptions } from 'vuex'
import { SquadOVUser } from '@client/js/squadov/user'

interface RootState {
    currentUser: SquadOVUser | null
}

export const RootStoreOptions : StoreOptions<RootState> = {
    strict: true,
    state: {
        currentUser: null,
    },
    mutations: {
        setUser(state : RootState, user : SquadOVUser) {
            state.currentUser = user
        }
    }
}