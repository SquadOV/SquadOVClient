import axios from 'axios'
import { apiClient, ApiData } from '@client/js/api' 

export interface SquadOVUser {
    id: number
    username: string
    email: string
    verified: boolean
    uuid: string
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
    return axios.get(`v1/users/${id}/profile`, apiClient.createWebAxiosConfig())
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
}

export function cleanUserProfileBasicFromJson(p: UserProfileBasic): UserProfileBasic {
    p.memberSince = new Date(p.memberSince)
    return p
}