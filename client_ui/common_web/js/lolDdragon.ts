const DDRAGON_BASE_URL = 'http://ddragon.leagueoflegends.com/cdn'
import axios from 'axios'

export interface LolDdragonChampionData {
    id: string
    key: string
    name: string
}

interface LolDdragonChampionRawData {
    data: { [key: string] : LolDdragonChampionData | undefined }
}

class LolDdragonChampionCache {
    rawData: LolDdragonChampionRawData
    keyData: Map<number, LolDdragonChampionData>

    constructor(data: LolDdragonChampionRawData) {
        this.rawData = data
        this.keyData = new Map()
        for (let [_, d] of Object.entries(this.rawData.data)) {
            this.keyData.set(parseInt(d!.key), d!)
        }
    }

    championDataForId(championId: number): LolDdragonChampionData | undefined {
        return this.keyData.get(championId)
    }
}

class LolDdragonClient {
    ddragonVersion : string

    // In-memory caches to store information such as the mapping of
    // champion ID to champions.
    championCache: LolDdragonChampionCache | null
    isLoadingChampionCache: boolean

    constructor(ddragonVersion: string) {
        this.ddragonVersion = ddragonVersion
        this.championCache = null
        this.isLoadingChampionCache = false
    }

    async loadChampionCache() {
        while (this.isLoadingChampionCache) {
            await new Promise((resolve) => {
                setTimeout(() => {
                    resolve(0)
                }, 100)
            })
        }

        if (!!this.championCache) {
            return
        }

        this.isLoadingChampionCache = true

        try {
            let data = await axios.get(`${DDRAGON_BASE_URL}/${this.ddragonVersion}/data/en_US/champion.json`)
            this.championCache = new LolDdragonChampionCache(data.data)
        } catch (e) {
            console.log('Failed to load Ddragon champion cache: ', e)
        }

        this.isLoadingChampionCache = false
    }

    getChampionIdFromKey(championId: number): Promise<string> {
        return new Promise(async (resolve, reject) => {
            if (!this.championCache) {
                await this.loadChampionCache()
            }

            if (!this.championCache) {
                reject('No Ddragon champion cache')
                return
            }

            let id = this.championCache?.championDataForId(championId)?.id
            if (!id) {
                reject(`No LoL champion with key: ${championId}`)
            } else {
                resolve(id)
            }
        })
    }

    getLolChampionIconUrl(championId: number): Promise<string> {
        return new Promise(async (resolve, reject) => {
            try {
                let key = await this.getChampionIdFromKey(championId)
                resolve(`${DDRAGON_BASE_URL}/${this.ddragonVersion}/img/champion/${key}.png`)
            } catch (e) {
                reject(`Failed to get champion icon: ${e}`)
            }
        })
    }
}

class LolDdragonClientContainer {
    clientCache: Map<string, LolDdragonClient>

    constructor () {
        this.clientCache = new Map()
    }

    getClientForVersion(version: string): LolDdragonClient {
        let tokens = version.split('.')
        let key = `${tokens[0]}.${tokens[1]}.1`
        if (!this.clientCache.has(key)) {
            this.clientCache.set(key, new LolDdragonClient(key))
        }
        return this.clientCache.get(key)!
    }
}

export let ddragonContainer = new LolDdragonClientContainer()