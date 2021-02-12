export interface Squad {
    id: number
    squadName: string
    squadGroup: string
    creationTime: Date
    memberCount: number
    pendingInviteCount: number
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