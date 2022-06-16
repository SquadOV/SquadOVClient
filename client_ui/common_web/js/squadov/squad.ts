import { SquadOVUserHandle } from "@client/js/squadov/user"
import { SquadOvGames, SquadOvWowRelease } from "@client/js/squadov/game"

export interface Squad {
    id: number
    squadName: string
    creationTime: Date
    memberCount: number
    pendingInviteCount: number
    maxMembers: number | null
}

export function cleanSquadFromJson(s : Squad) : Squad {
    s.creationTime = new Date(s.creationTime)
    return s
}

export enum SquadRole {
    Owner,
    Member
}

export interface SquadMembership {
    squad: Squad
    role: SquadRole
    username: string
    userId: number
    canShare: boolean
}

export function cleanSquadMembershipFromJson(s: SquadMembership): SquadMembership {
    cleanSquadFromJson(s.squad)

    //@ts-ignore
    s.role = SquadRole[s.role]
    return s
}

export interface SquadInvite {
    squadId: number
    username: string | null
    // Not sure how I feel about sending the email here.
    email: string
    userId: number
    joined: boolean
    responseTime: Date | null
    inviteTime: Date | null
    inviteUuid: string
    inviterUsername: string
}

export function cleanSquadInviteFromJson(s : SquadInvite) : SquadInvite {
    if (!!s.responseTime) {
        s.responseTime = new Date(s.responseTime)
    }

    if (!!s.inviteTime) {
        s.inviteTime = new Date(s.inviteTime)
    }
    return s
}

export interface SquadInviteLink {
    id: string
    squadId: number
    userId: number
    createTime: Date
    expireTime: Date | null
    useCount: number
    maxUses: number | null
    link: string
}

export function cleanSquadInviteLinkFromJson(s: SquadInviteLink) : SquadInviteLink {
    s.createTime = new Date(s.createTime)
    if (!!s.expireTime) {
        s.expireTime = new Date(s.expireTime)
    }
    return s
}

export interface SquadInviteLinkData {
    squad: Squad
    inviter: SquadOVUserHandle
    referral: string
}

export function cleanSquadInviteLinkDataFromJson(s: SquadInviteLinkData) : SquadInviteLinkData {
    cleanSquadFromJson(s.squad)
    return s
}

export interface SquadWowSharingSettings {
    disabledReleases: SquadOvWowRelease[]
    disableEncounters: boolean
    disableDungeons: boolean
    disableKeystones: boolean
    disableArenas: boolean
    disableBgs: boolean
}

export interface SquadSharingSettings {
    disabledGames: SquadOvGames[]
    wow: SquadWowSharingSettings
}