import axios from 'axios'
import { apiClient, ApiData } from '@client/js/api' 

export interface SquadOVUser {
    id: number
    username: string
    email: string
    verified: boolean
    uuid: string
}

export interface SquadOvHeartbeatResponse {
    sessionId: string
    expiration: Date
}

export function cleanSquadOvHeartbeatResponse(s: SquadOvHeartbeatResponse) : SquadOvHeartbeatResponse {
    s.expiration = new Date(s.expiration)
    return s
}

export function getSquadOVUser(id : number) : Promise<ApiData<SquadOVUser>> {
    return axios.get(`v1/users/${id}/profile`, apiClient.createWebAxiosConfig())
}