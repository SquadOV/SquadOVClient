import axios from 'axios'

import { ValorantAccountData } from '@client/js/valorant/valorant_account'
import { ValorantPlayerStatsSummary } from '@client/js/valorant/valorant_player_stats'
import {
    ValorantPlayerMatchSummary,
    ValorantMatchDetails,
    cleanValorantPlayerMatchSummary,
    cleanValorantMatchDetails,
} from '@client/js/valorant/valorant_matches'
import { AimlabTaskData, cleanAimlabTaskData } from '@client/js/aimlab/aimlab_task'
import { GraphqlQuery } from '@client/js/graphql/graphql'

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
    sessionId: string
}

interface RegisterInput {
    username: string
    password: string
    email: string
}

class ApiClient {
    createAxiosConfig(endpoint : string) : any {
        return {
            url: `https://127.0.0.1:${process.env.SQUADOV_API_PORT}/${endpoint}`,
            headers: {
                'Authorization': `Bearer ${process.env.SQUADOV_API_KEY}`
            },
        }
    }

    createWebAxiosConfig() : any {
        return {
            baseURL: API_URL,
        }
    }

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

    @waitForApiServerSetup
    listValorantMatchesForPlayer(puuid : string) : Promise<ApiData<ValorantPlayerMatchSummary[]>> {
        return axios({
            ...this.createAxiosConfig(`valorant/accounts/${puuid}/matches`),
            method: 'get'
        }).then((resp : ApiData<ValorantPlayerMatchSummary[]>) => {
            resp.data.forEach(cleanValorantPlayerMatchSummary)
            return resp
        })
    }

    @waitForApiServerSetup
    getValorantPlayerStats(puuid : string) : Promise<ApiData<ValorantPlayerStatsSummary>> {
        return axios({
            ...this.createAxiosConfig(`valorant/stats/summary/${puuid}`),
            method: 'get'
        })
    }

    @waitForApiServerSetup
    getValorantMatchDetails(matchId : string) : Promise<ApiData<ValorantMatchDetails>> {
        return axios({
            ...this.createAxiosConfig(`valorant/matches/${matchId}`),
            method: 'get'
        }).then((resp : ApiData<ValorantMatchDetails>) => {
            cleanValorantMatchDetails(resp.data)
            return resp
        })
    }

    @waitForApiServerSetup
    allAimlabTaskData() : Promise<ApiData<AimlabTaskData[]>> {
        return axios({
            ...this.createAxiosConfig(`aimlab/tasks`),
            method: 'get'
        }).then((resp : ApiData<AimlabTaskData[]>) => {
            resp.data.forEach(cleanAimlabTaskData)
            return resp
        })
    }

    @waitForApiServerSetup
    getAimlabTaskData(id : number) : Promise<ApiData<AimlabTaskData>> {
        return axios({
            ...this.createAxiosConfig(`aimlab/tasks/${id}`),
            method: 'get'
        }).then((resp : ApiData<AimlabTaskData>) => {
            cleanAimlabTaskData(resp.data)
            return resp
        })
    }

    @waitForApiServerSetup
    graphqlRequest(req : GraphqlQuery) : Promise<GraphqlApiData<any>> {
        let baseConfig : any = this.createAxiosConfig(`graphql`)
        baseConfig.method = 'post'
        baseConfig.data = req.generateBody()
        baseConfig.headers['Content-Type'] = 'application/graphql'
        return axios(baseConfig)
    }

    login(inp : LoginInput) : Promise<LoginOutput> {
        return axios.post('auth/login', inp, this.createWebAxiosConfig())
    }

    register(inp : RegisterInput) : Promise<void> {
        return axios.post('auth/register', inp, this.createWebAxiosConfig())
    }
}

export let apiClient = new ApiClient()