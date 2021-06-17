export interface CommunityFilter {
    onlyMe?: boolean
}

export enum CommunitySecurityLevel {
    Public,
    Private,
    Unlisted
}

export function communitySecurityLevelToString(s: CommunitySecurityLevel): string {
    switch (s) {
        case CommunitySecurityLevel.Public:
            return 'Public'
        case CommunitySecurityLevel.Private:
            return 'Private'
        case CommunitySecurityLevel.Unlisted:
            return 'Unlisted'
    }
}

export interface SquadOvCommunity {
    id: number
    name: string
    slug: string
    createTm: Date
    creatorUserId: number
    securityLevel: CommunitySecurityLevel
    requiresSubscription: boolean
    allowTwitchSub: boolean
}

export function createDefaultCommunity(): SquadOvCommunity {
    return {
        id: -1,
        name: '',
        slug: '',
        createTm: new Date(),
        creatorUserId: -1,
        securityLevel: CommunitySecurityLevel.Public,
        requiresSubscription: false,
        allowTwitchSub: true,
    }
}

export function cleanCommunityFromJson(c: SquadOvCommunity): SquadOvCommunity {
    c.createTm = new Date(c.createTm)
    return c
}

export interface CommunityRole {
    id: number
    communityId: number
    name: string
    canManage: boolean
    canModerate: boolean
    canInvite: boolean
    canShare: boolean
    isDefault: boolean
}