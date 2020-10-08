import axios from 'axios'
import { apiClient, ApiData } from '@client/js/api' 

export interface SquadOVUser {
    id: number
    username: string
    email: string
}

export function getSquadOVUser(id : number) : Promise<ApiData<SquadOVUser>> {
    return axios.get(`v1/users/${id}/profile`, apiClient.createWebAxiosConfig())
}