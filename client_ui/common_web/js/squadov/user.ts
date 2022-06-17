import axios from 'axios'
import { apiClient, ApiData } from '@client/js/api' 

export interface SquadOVUser {
    id: number
    username: string
    email: string
    verified: boolean
    uuid: string
    supportPriority: string
    lastTrialUsage: Date | null
}

export function canUserTrial(u: SquadOVUser): boolean {
    if (!u.lastTrialUsage) {
        return true
    }

    let diff = new Date().getTime() - u.lastTrialUsage.getTime()
    return ((diff / 86400000) >= 365)
}

export function cleanSquadOVUserFromJson(u: SquadOVUser): SquadOVUser {
    if (!!u.lastTrialUsage) {
        u.lastTrialUsage = new Date(u.lastTrialUsage)
    }
    return u
}

export interface SquadOVUserHandle {
    id: number
    username: string
}

export interface SquadOvHeartbeatResponse {
    sessionId: string
    expiration: Date
    expiresIn: number
}

export function cleanSquadOvHeartbeatResponse(s: SquadOvHeartbeatResponse) : SquadOvHeartbeatResponse {
    s.expiration = new Date(s.expiration)
    return s
}

export function getSquadOVUser(id : number) : Promise<ApiData<SquadOVUser>> {
    return axios.get(`v1/users/${id}/profile`, apiClient.createWebAxiosConfig()).then((resp: ApiData<SquadOVUser>) => {
        cleanSquadOVUserFromJson(resp.data)
        return resp
    })
}

export interface UserProfileBasic {
    userId: number
    linkSlug: string
    username: string
    displayName: string
    misc: {
        description: string
        profilePictureUrl: string | null
        coverPictureUrl: string | null
        links: {
            facebook: string | null
            instagram: string | null
            twitch: string | null
            youtube: string | null
            snapchat: string | null
            twitter: string | null
            tiktok: string | null
        }
    } | null
    hasAchievementAccess: boolean
    hasMatchAccess: boolean
    memberSince: Date
    miscAccess: number
    achievementAccess: number
    matchAccess: number
    twitchChannelForSub: string | null
    accessToken: string | null
}

export interface UserProfileHandle {
    username: string
    slug: string
}

export function cleanUserProfileBasicFromJson(p: UserProfileBasic): UserProfileBasic {
    p.memberSince = new Date(p.memberSince)
    return p
}

export const USER_PROFILE_ACCESS_PRIVATE_SQUADS: number = 1
export const USER_PROFILE_ACCESS_PRIVATE_TWITCH_SUB: number = 2
export const USER_PROFILE_ACCESS_PRIVATE_SQUADOV_SUB: number = 4
export const USER_PROFILE_ACCESS_PUBLIC: number = 8