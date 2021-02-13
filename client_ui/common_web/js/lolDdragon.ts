const DDRAGON_BASE_URL = 'http://ddragon.leagueoflegends.com/cdn'
import axios from 'axios'
import { ApiData } from '@client/js/api'

export interface LolDdragonData {
    key: string | undefined
    id: string | undefined
    name: string | undefined
}

export interface LolDdragonChampionData extends LolDdragonData {
    
}

export interface LolDdragonSummonerSpellData extends LolDdragonData {
}

export interface LolDdragonItemData extends LolDdragonData {
}

interface LolDdragonRawData<T> {
    data: { [key: string] : T | undefined }
}

class LolDdragonCache<TParsed extends LolDdragonData> {
    rawData: LolDdragonRawData<TParsed>
    keyData: Map<number, TParsed>

    constructor(data: LolDdragonRawData<TParsed>) {
        this.rawData = data
        this.keyData = new Map()
        for (let [_, d] of Object.entries(this.rawData.data)) {
            if (!d?.key) {
                continue
            }
            this.keyData.set(parseInt(d.key), d!)
        }
    }

    dataForId(id: number): TParsed | undefined {
        return this.keyData.get(id)
    }

    dataForKey(key: string): TParsed | undefined {
        return this.rawData.data[key]
    }
}

class LolDdragonClient {
    ddragonVersion : string

    // In-memory caches to store information such as the mapping of
    // champion ID to champions.
    championCache: LolDdragonCache<LolDdragonChampionData> | null
    isLoadingChampionCache: boolean

    summonerSpellCache: LolDdragonCache<LolDdragonSummonerSpellData> | null
    isLoadingSummonerSpellCache: boolean

    itemCache: LolDdragonCache<LolDdragonItemData> | null
    isLoadingItemCache: boolean

    constructor(ddragonVersion: string) {
        this.ddragonVersion = ddragonVersion
        this.championCache = null
        this.isLoadingChampionCache = false
        this.summonerSpellCache = null
        this.isLoadingSummonerSpellCache = false
        this.itemCache = null
        this.isLoadingItemCache = false
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
            this.championCache = new LolDdragonCache(data.data)
        } catch (e) {
            console.log('Failed to load Ddragon champion cache: ', e)
        }

        this.isLoadingChampionCache = false
    }

    async loadSummonerSpellCache() {
        while (this.isLoadingSummonerSpellCache) {
            await new Promise((resolve) => {
                setTimeout(() => {
                    resolve(0)
                }, 100)
            })
        }

        if (!!this.summonerSpellCache) {
            return
        }

        this.isLoadingSummonerSpellCache = true

        try {
            let data = await axios.get(`${DDRAGON_BASE_URL}/${this.ddragonVersion}/data/en_US/summoner.json`)
            this.summonerSpellCache = new LolDdragonCache(data.data)
        } catch (e) {
            console.log('Failed to load Ddragon summoner spell cache: ', e)
        }

        this.isLoadingSummonerSpellCache = false
    }

    async loadItemCache() {
        while (this.isLoadingItemCache) {
            await new Promise((resolve) => {
                setTimeout(() => {
                    resolve(0)
                }, 100)
            })
        }

        if (!!this.itemCache) {
            return
        }

        this.isLoadingItemCache = true

        try {
            let data = await axios.get(`${DDRAGON_BASE_URL}/${this.ddragonVersion}/data/en_US/item.json`)
            this.itemCache = new LolDdragonCache(data.data)
        } catch (e) {
            console.log('Failed to load Ddragon Item cache: ', e)
        }

        this.isLoadingItemCache = false
    }

    getChampionDataFromKey(championId: number): Promise<LolDdragonChampionData | undefined> {
        return new Promise(async (resolve, reject) => {
            if (!this.championCache) {
                await this.loadChampionCache()
            }

            if (!this.championCache) {
                reject('No Ddragon champion cache')
                return
            }

            resolve(this.championCache?.dataForId(championId))
        })
    }

    getSummonerSpellDataFromKey(spellId: number): Promise<LolDdragonSummonerSpellData | undefined> {
        return new Promise(async (resolve, reject) => {
            if (!this.summonerSpellCache) {
                await this.loadSummonerSpellCache()
            }

            if (!this.summonerSpellCache) {
                reject('No Ddragon summoner spell cache')
                return
            }

            resolve(this.summonerSpellCache?.dataForId(spellId))
        })
    }

    getItemDataFromKey(itemId: number): Promise<LolDdragonItemData | undefined> {
        return new Promise(async (resolve, reject) => {
            if (!this.itemCache) {
                await this.loadItemCache()
            }

            if (!this.itemCache) {
                reject('No Ddragon item cache')
                return
            }

            resolve(this.itemCache?.dataForKey(`${itemId}`))
        })
    }

    getChampionIdFromKey(championId: number): Promise<string> {
        return new Promise(async (resolve, reject) => {
            let cache = await this.getChampionDataFromKey(championId)
            let id = cache?.id
            if (!id) {
                reject(`No LoL champion with key: ${championId}`)
            } else {
                resolve(id)
            }
        })
    }

    getSummonerSpellIdFromKey(spellId: number): Promise<string> {
        return new Promise(async (resolve, reject) => {
            let cache = await this.getSummonerSpellDataFromKey(spellId)
            let id = cache?.id
            if (!id) {
                reject(`No LoL summoner spell with key: ${spellId}`)
            } else {
                resolve(id)
            }
        })
    }

    getLolChampionName(championId: number): Promise<string> {
        return new Promise(async (resolve, reject) => {
            try {
                let cache = await this.getChampionDataFromKey(championId)
                resolve(!!cache?.name ? cache.name : 'Unknown')
            } catch (e) {
                reject(`Failed to get champion name: ${e}`)
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

    getLolChampionSplashUrl(championId: number): Promise<string> {
        return new Promise(async (resolve, reject) => {
            try {
                let key = await this.getChampionIdFromKey(championId)
                resolve(`${DDRAGON_BASE_URL}/img/champion/splash/${key}_0.jpg`)
            } catch (e) {
                reject(`Failed to get champion splash: ${e}`)
            }
        })
    }

    getLolSummonerSpellIconUrl(spellId: number): Promise<string> {
        return new Promise(async (resolve, reject) => {
            try {
                let key = await this.getSummonerSpellIdFromKey(spellId)
                resolve(`${DDRAGON_BASE_URL}/${this.ddragonVersion}/img/spell/${key}.png`)
            } catch (e) {
                reject(`Failed to get champion icon: ${e}`)
            }
        })
    }

    getLolItemIconUrl(itemId: number): string {
        return `${DDRAGON_BASE_URL}/${this.ddragonVersion}/img/item/${itemId}.png`
    }

    getLolItemName(itemId: number): Promise<string> {
        return new Promise(async (resolve, reject) => {
            try {
                let cache = await this.getItemDataFromKey(itemId)
                resolve(!!cache?.name ? cache.name : 'Unknown')
            } catch (e) {
                reject(`Failed to get item name: ${e}`)
            }
        })
    }
}

class LolDdragonClientContainer {
    clientCache: Map<string, LolDdragonClient>
    validVersions: Set<string>
    defaultVersion: string = ''

    constructor () {
        this.clientCache = new Map()
        this.validVersions = new Set()
        this.refreshValidVersions()
    }

    refreshValidVersions() {
        axios.get(`http://ddragon.leagueoflegends.com/api/versions.json`).then((resp: ApiData<string[]>) => {
            resp.data.forEach((ele: string) => this.validVersions.add(ele))
            this.defaultVersion = resp.data[0]
        }).catch((err: any) => {
            console.log('Failed to obtain DDragon version: ', err)
        })
    }

    getClientForVersion(version: string): LolDdragonClient {
        let tokens = version.split('.')
        let key = `${tokens[0]}.${tokens[1]}.1`

        if (!this.validVersions.has(key)) {
            key = this.defaultVersion
        }

        if (!this.clientCache.has(key)) {
            this.clientCache.set(key, new LolDdragonClient(key))
        }
        return this.clientCache.get(key)!
    }
}

export let ddragonContainer = new LolDdragonClientContainer()