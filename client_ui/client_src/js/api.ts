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

const waitForApiServerSetup = (target : any , key : any, descriptor : any) => {
    const ogMethod = descriptor.value
    descriptor.value = async function(...args : any[]) {
        let port = process.env.SQUADOV_API_PORT
        let key = process.env.SQUADOV_API_KEY

        while (port === undefined || key === undefined) {
            console.log(`Could not find port or key for SQUADOV service...port ${port} key ${key}`)
            port = process.env.SQUADOV_API_PORT
            key = process.env.SQUADOV_API_KEY
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

class ApiClient {
    createAxiosConfig(endpoint : string) : any {
        return {
            url: `http://127.0.0.1:${process.env.SQUADOV_API_PORT}/${endpoint}`,
            headers: {
                'Authorization': `Bearer ${process.env.SQUADOV_API_KEY}`
            }
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
}

export let apiClient = new ApiClient()