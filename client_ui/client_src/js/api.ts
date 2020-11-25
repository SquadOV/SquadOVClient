import axios from 'axios'

import { ValorantAccountData } from '@client/js/valorant/valorant_account'
import { ValorantPlayerStatsSummary } from '@client/js/valorant/valorant_player_stats'
import {
    ValorantPlayerMatchSummary,
    ValorantMatchDetails,
    cleanValorantPlayerMatchSummary,
    cleanValorantMatchDetails,
    ValorantMatchPlayerMatchMetadata,
    cleanValorantMatchPlayerMatchMetadata,
} from '@client/js/valorant/valorant_matches'
import { AimlabTaskData, cleanAimlabTaskData } from '@client/js/aimlab/aimlab_task'
import { VodAssociation, cleanVodAssocationData, VodManifest } from '@client/js/squadov/vod'
import { HearthstoneMatch, HearthstoneMatchLogs, cleanHearthstoneMatchFromJson, cleanHearthstoneMatchLogsFromJson } from '@client/js/hearthstone/hearthstone_match'
import { HearthstoneCardMetadata } from '@client/js/hearthstone/hearthstone_deck'
import { HearthstoneGameType } from '@client/js/hearthstone/hearthstone_match'
import { HearthstoneArenaRun, cleanHearthstoneArenaRunFromJson } from '@client/js/hearthstone/hearthstone_arena'
import { HearthstoneDuelRun, cleanHearthstoneDuelRunFromJson } from '@client/js/hearthstone/hearthstone_duel'

import { ipcRenderer } from 'electron'

const waitForApiServerSetup = (target : any , key : any, descriptor : any) => {
    const ogMethod = descriptor.value
    descriptor.value = async function(...args : any[]) {
        let port = process.env.SQUADOV_API_PORT
        let key = process.env.SQUADOV_API_KEY

        while (port === undefined || key === undefined) {
            console.log(`Could not find port or key for SQUADOV service...port ${port} key ${key}`)
            
            // Need to listen to port/key messages from the main process just in case that gets set late...
            const [newPort, newKey] = ipcRenderer.sendSync('apiServer')
            port = process.env.SQUADOV_API_PORT = newPort
            key = process.env.SQUADOV_API_KEY = newKey

            await new Promise(resolve => setTimeout(resolve, 100))
        }

        return ogMethod.apply(this, args)
    }

    return {
        ...descriptor,
        initializer: undefined
    }
}

export interface ApiData<T> {
    data: T
}

export interface HalLink {
    href: string
}

export interface HalResponse<T> {
    _links: {
        [index : string] : HalLink
    },
    data: T
}

export interface GraphqlApiData<T> {
    data: {
        data: T
    }
}

interface LoginInput {
    username: string
    password: string
}

export interface LoginOutput {
    userId: number
    sessionId: string
    verified: boolean
}

interface RegisterInput {
    username: string
    password: string
    email: string
}

export interface CheckVerificationOutput {
    verified: boolean
}

class ApiClient {
    _sessionId: string | null

    constructor() {
        this._sessionId = null
    }

    setSessionId(s : string) {
        this._sessionId = s
    }

    createAxiosConfig(endpoint : string) : any {
        return {
            url: `https://127.0.0.1:${process.env.SQUADOV_API_PORT}/${endpoint}`,
            headers: {
                'Authorization': `Bearer ${process.env.SQUADOV_API_KEY}`
            },
        }
    }

    createWebAxiosConfig() : any {
        let ret : any = {
            baseURL: process.env.API_SQUADOV_URL,
        }

        if (!!this.setSessionId) {
            ret.headers = {
                'x-squadov-session-id': this._sessionId,
            }
        }

        return ret
    }

    //
    // Legay Local API
    //
    @waitForApiServerSetup
    listValorantAccounts() : Promise<ApiData<ValorantAccountData[]>> {
        return axios({
            ...this.createAxiosConfig('valorant/accounts'),
            method: 'get'
        })
    }

    @waitForApiServerSetup
    editValorantAccount(puuid : string, login : string, password : string) : Promise<ApiData<ValorantAccountData>> {
        return axios({
            ...this.createAxiosConfig(`valorant/accounts/${puuid}`),
            method: 'put',
            data: {
                login,
                password
            }
        })
    }

    @waitForApiServerSetup
    newValorantAccount(login : string, password : string) : Promise<ApiData<ValorantAccountData>> {
        return axios({
            ...this.createAxiosConfig('valorant/accounts'),
            method: 'post',
            data: {
                login,
                password
            }
        })
    }

    @waitForApiServerSetup
    getValorantAccount(puuid : string) : Promise<ApiData<ValorantAccountData>> {  
        return axios({
            ...this.createAxiosConfig(`valorant/accounts/${puuid}`),
            method: 'get'
        })
    }

    //
    // Web server API
    //
    login(inp : LoginInput) : Promise<ApiData<LoginOutput>> {
        return axios.post('auth/login', inp, this.createWebAxiosConfig())
    }

    register(inp : RegisterInput) : Promise<void> {
        return axios.post('auth/register', inp, this.createWebAxiosConfig())
    }

    resendVerification() : Promise<void> {
        return axios.post('auth/verify/resend', {}, this.createWebAxiosConfig())
    }

    checkVerification() : Promise<ApiData<CheckVerificationOutput>> {
        return axios.get('auth/verify', this.createWebAxiosConfig())
    }

    forgotPassword(loginId : string) : Promise<void> {
        return axios.get('auth/forgotpw', {
            ...this.createWebAxiosConfig(),
            params: {
                loginId,
            }
        })
    }

    logout() : Promise<void> {
        return axios.post('auth/logout', {}, this.createWebAxiosConfig())
    }

    findVodFromMatchUserUuid(matchUuid : string, userUuid: string) : Promise<ApiData<VodAssociation>> {
        return axios.get(`v1/vod/match/${matchUuid}/user/${userUuid}`, this.createWebAxiosConfig()).then((resp : ApiData<VodAssociation>) => {
            cleanVodAssocationData(resp.data)
            return resp
        })
    }

    getVodManifest(videoUuid: string) : Promise<ApiData<VodManifest>> {
        return axios.get(`v1/vod/${videoUuid}`, this.createWebAxiosConfig())
    }

    getVodSegment(url : string) : Promise<ApiData<string>> {
        return axios.get(url, {
            ...this.createWebAxiosConfig(),
        })
    }

    allAimlabTaskData(params : {next : string | null, userId : number, start : number, end : number}) : Promise<ApiData<HalResponse<AimlabTaskData[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/aimlab/user/${params.userId!}`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                }
            })

        return promise.then((resp : ApiData<HalResponse<AimlabTaskData[]>>) => {
            resp.data.data.forEach(cleanAimlabTaskData)
            return resp
        })
    }

    getAimlabTaskData(uuid : string) : Promise<ApiData<AimlabTaskData>> {
        return axios.get(`v1/aimlab/match/${uuid}/task`, this.createWebAxiosConfig()).then((resp : ApiData<AimlabTaskData>) => {
            cleanAimlabTaskData(resp.data)
            return resp
        })
    }

    listValorantMatchesForPlayer(params : {next : string | null, puuid : string, start : number, end : number}) : Promise<ApiData<HalResponse<ValorantPlayerMatchSummary[]>>> {
        let promise = !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/valorant/accounts/${params.puuid}/matches`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start!,
                    end: params.end!,
                }
            })

        return promise.then((resp : ApiData<HalResponse<ValorantPlayerMatchSummary[]>>) => {
            resp.data.data.forEach(cleanValorantPlayerMatchSummary)
            return resp
        })
    }

    getValorantPlayerStats(puuid : string) : Promise<ApiData<ValorantPlayerStatsSummary>> {
        return axios.get(`v1/valorant/accounts/${puuid}/stats`, this.createWebAxiosConfig())
    }

    
    getValorantMatchDetails(matchId : string) : Promise<ApiData<ValorantMatchDetails>> {
        return axios.get(`v1/valorant/match/${matchId}`, this.createWebAxiosConfig()).then((resp : ApiData<ValorantMatchDetails>) => {
            cleanValorantMatchDetails(resp.data)
            return resp
        })
    }

    getValorantMatchPlayerMetadata(matchId: string, puuid: string) : Promise<ApiData<ValorantMatchPlayerMatchMetadata>> {
        return axios.get(`v1/valorant/match/${matchId}/metadata/${puuid}`, this.createWebAxiosConfig()).then((resp : ApiData<ValorantMatchPlayerMatchMetadata>) => {
            cleanValorantMatchPlayerMatchMetadata(resp.data)
            return resp
        })
    }

    listHearthstoneMatchesForPlayer(params : {next : string | null, userId : number, start : number, end : number, filter : HearthstoneGameType[]}) : Promise<ApiData<HalResponse<string[]>>> {
        return !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/hearthstone/user/${params.userId!}/match`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start,
                    end: params.end,
                    filter: JSON.stringify(params.filter)
                }
            })
    }

    listHearthstoneMatchesForArenaRun(collectionUuid: string, userId: number): Promise<ApiData<string[]>> {
        return axios.get(`v1/hearthstone/user/${userId}/arena/${collectionUuid}/matches`, this.createWebAxiosConfig())
    }

    listHearthstoneMatchesForDuelRun(collectionUuid: string, userId: number): Promise<ApiData<string[]>> {
        return axios.get(`v1/hearthstone/user/${userId}/duels/${collectionUuid}/matches`, this.createWebAxiosConfig())
    }

    listHearthstoneArenaRunsForPlayer(params : {next : string | null, userId : number, start : number, end : number}) : Promise<ApiData<HalResponse<string[]>>> {
        return !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/hearthstone/user/${params.userId!}/arena`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start,
                    end: params.end,
                }
            })
    }

    listHearthstoneDuelRunsForPlayer(params : {next : string | null, userId : number, start : number, end : number}) : Promise<ApiData<HalResponse<string[]>>> {
        return !!params.next ?
            axios.get(params.next, this.createWebAxiosConfig()) :
            axios.get(`v1/hearthstone/user/${params.userId!}/duels`, {
                ...this.createWebAxiosConfig(),
                params: {
                    start: params.start,
                    end: params.end,
                }
            })
    }

    getHearthstoneDuelRun(collectionUuid: string, userId: number): Promise<ApiData<HearthstoneDuelRun>> {
        return axios.get(`v1/hearthstone/user/${userId}/duels/${collectionUuid}`, this.createWebAxiosConfig()).then((resp: ApiData<HearthstoneDuelRun>) => {
            cleanHearthstoneDuelRunFromJson(resp.data)
            return resp
        })
    }

    getHearthstoneArenaRun(collectionUuid: string, userId: number): Promise<ApiData<HearthstoneArenaRun>> {
        return axios.get(`v1/hearthstone/user/${userId}/arena/${collectionUuid}`, this.createWebAxiosConfig()).then((resp: ApiData<HearthstoneArenaRun>) => {
            cleanHearthstoneArenaRunFromJson(resp.data)
            return resp
        })
    }

    getHearthstoneMatch(matchId: string) : Promise<ApiData<HearthstoneMatch>> {
        return axios.get(`v1/hearthstone/match/${matchId}`, this.createWebAxiosConfig()).then((resp : ApiData<HearthstoneMatch>) => {
            cleanHearthstoneMatchFromJson(resp.data)
            return resp
        })
    }

    getHearthstoneMatchLogs(matchId: string) : Promise<ApiData<HearthstoneMatchLogs>> {
        return axios.get(`v1/hearthstone/match/${matchId}/logs`, this.createWebAxiosConfig()).then((resp: ApiData<HearthstoneMatchLogs>) => {
            cleanHearthstoneMatchLogsFromJson(resp.data)
            return resp
        })
    }

    getBulkHearthstoneCardMetadata(cards: string[]) : Promise<ApiData<HearthstoneCardMetadata[]>> {
        return axios.post(`v1/hearthstone/cards`, cards, this.createWebAxiosConfig())
    }

    graphqlRequest(req : string) : Promise<GraphqlApiData<any>> {
        return axios.post('graphql', {
            query: req
        }, this.createWebAxiosConfig())
    }
}

export let apiClient = new ApiClient()

const sessionSetHeader = 'x-squadov-set-session-id'
function parseResponseHeaders(headers : any) {
    if (!(sessionSetHeader in headers)) {
        return
    }

    const newSessionId = headers[sessionSetHeader]
    apiClient.setSessionId(newSessionId)

    // Also need to notify the main process so that it can communicate with the local service somehow.
    ipcRenderer.send('refresh-session', newSessionId)
}

ipcRenderer.on('update-session', (event : any, message : string) => {
    apiClient.setSessionId(message)
})

axios.interceptors.response.use((resp) => {
    parseResponseHeaders(resp.headers)
    return resp
}, (err) => {
    parseResponseHeaders(err.response.headers)
    return Promise.reject(err)
})
