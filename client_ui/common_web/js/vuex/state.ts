import { SquadOvState } from '@client/js/system/state'
import { SquadOvLocalSettings } from '@client/js/system/settings'
import { TrackedUserStatusContainer } from '@client/js/squadov/status'
import { FeatureFlags } from '@client/js/squadov/features'
import { SquadOVUser } from '@client/js/squadov/user'
import { EPricingTier } from '@client/js/squadov/pricing'

export interface RootState {
    currentUser: SquadOVUser | null
    tier: EPricingTier | null
    redirectUrl: string | null
    hasValidSession: boolean
    attemptUserLoad: boolean
    features: FeatureFlags
/// #if DESKTOP
    settings: SquadOvLocalSettings | null
    currentState: SquadOvState
    localDiskSpaceRecordUsageGb: number | null
/// #endif
    status: TrackedUserStatusContainer
    hasUpdate: boolean
    muteInviteFriendsPopUp: boolean
    serviceError: boolean
    successfullyVisitedVideo: boolean
    displayInviteFriendPopUp: boolean
    forceHideNav: boolean
    cachedIp: string | null
    requiresRestart: boolean
}
