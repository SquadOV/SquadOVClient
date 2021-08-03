import { SquadOvState } from '@client/js/system/state'
import { SquadOvLocalSettings } from '@client/js/system/settings'
import { TrackedUserStatusContainer } from '@client/js/squadov/status'
import { FeatureFlags } from '@client/js/squadov/features'
import { SquadOVUser } from '@client/js/squadov/user'

export interface RootState {
    currentUser: SquadOVUser | null
    redirectUrl: string | null
    hasValidSession: boolean
    features: FeatureFlags
/// #if DESKTOP
    settings: SquadOvLocalSettings | null
    currentState: SquadOvState
/// #endif
    status: TrackedUserStatusContainer
}
