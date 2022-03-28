import axios from 'axios'
import { staticClient, getSupportedWowPatch, WowGameRelease } from '@client/js/staticData'
import { ApiData } from '@client/js/api'

export const LATEST_RETAIL_PATCH_ID = '9.2.0'

interface WowBasicStatic {
    id: string
    name: string
}

export interface WowConduitStatic {
    id: number
    itemId: number
}

export interface WowItemStatic {
    id: number 
    name: string
    quality: number
    inventorySlot: number
}

export interface WowClassSpecStatic {
    id: number
    name: string
    class: number
}

export interface WowClassStatic {
    id: number
    name: string
}

export interface WowTalentStatic {
    id: number
    name: string
    socket: number
}

export interface WowConduitStatic {
    id: number
    itemId: number
}

class WowStaticCache {
    _spellNames: Map<number, string>
    _items: Map<number, WowItemStatic>
    _specs: Map<number, WowClassSpecStatic>
    _classes: Map<number, WowClassStatic>
    _covenants: Map<number, string>
    _soulbinds: Map<number, string>
    _conduits: Map<number, number>
    _talents: Map<number, WowTalentStatic>
    _patch: string

    constructor(patch: string) {
        this._patch = patch
        this._spellNames = new Map()
        this._items = new Map()
        this._specs = new Map()
        this._classes = new Map()
        this._covenants = new Map()
        this._soulbinds = new Map()
        this._conduits = new Map()
        this._talents = new Map()
    }

    async getCovenant(covenantId: number): Promise<string> {
        if (this._covenants.has(covenantId)) {
            return this._covenants.get(covenantId)!
        }
        
        let data: ApiData<WowBasicStatic> = await axios.get(staticClient.getWowCovenantDataUrl(this._patch, covenantId))
        this._covenants.set(covenantId, data.data.name)
        return data.data.name
    }

    async getSoulbind(soulbindId: number): Promise<string> {
        if (this._soulbinds.has(soulbindId)) {
            return this._soulbinds.get(soulbindId)!
        }
        
        let data: ApiData<WowBasicStatic> = await axios.get(staticClient.getWowSoulbindDataUrl(this._patch, soulbindId))
        this._soulbinds.set(soulbindId, data.data.name)
        return data.data.name
    }

    async getConduitItem(conduitId: number): Promise<number> {
        if (this._conduits.has(conduitId)) {
            return this._conduits.get(conduitId)!
        }
        
        let data: ApiData<WowConduitStatic> = await axios.get(staticClient.getWowConduitDataUrl(this._patch, conduitId))
        this._conduits.set(conduitId, data.data.itemId)
        return data.data.itemId
    }
    
    async getTalent(talentId: number): Promise<WowTalentStatic> {
        if (this._talents.has(talentId)) {
            return this._talents.get(talentId)!
        }

        let data: ApiData<WowTalentStatic> = await axios.get(staticClient.getWowTalentDataUrl(this._patch, talentId))
        this._talents.set(talentId, data.data)
        return data.data
    }

    async getClassSpec(specId: number): Promise<WowClassSpecStatic> {
        if (this._specs.has(specId)) {
            return this._specs.get(specId)!
        }

        let data: ApiData<WowClassSpecStatic> = await axios.get(staticClient.getWowSpecsDataUrl(this._patch, specId))
        this._specs.set(specId, data.data)
        return data.data
    }

    async getClass(classId: number): Promise<WowClassStatic> {
        if (this._classes.has(classId)) {
            return this._classes.get(classId)!
        }

        let data: ApiData<WowClassStatic> = await axios.get(staticClient.getWowClassDataUrl(this._patch, classId))
        this._classes.set(classId, data.data)
        return data.data
    }

    async bulkGetSpellNames(spellIds: number[]): Promise<Map<number, string>> {
        let ret: Map<number, string> = new Map()

        let newIds = spellIds.filter((id: number) => !this._spellNames.has(id))
        if (newIds.length > 0) {
            let data: PromiseSettledResult<ApiData<WowBasicStatic>>[] = await Promise.allSettled(newIds.map((id: number) => {
                return axios.get(staticClient.getWowSpellDataUrl(this._patch, id))
            }))

            for (let d of data) {
                if (d.status === 'rejected') {
                    continue
                }
                this._spellNames.set(parseInt(d.value.data.id), d.value.data.name)
            }
        }

        spellIds.forEach((id: number) => {
            ret.set(id, this._spellNames.get(id)!)
        })
        return ret
    }

    async bulkGetItems(itemIds: number[]): Promise<Map<number, WowItemStatic>> {
        let ret: Map<number, WowItemStatic> = new Map()

        let newIds = itemIds.filter((id: number) => !this._items.has(id) && id !== 0)
        if (newIds.length > 0) {
            let data: PromiseSettledResult<ApiData<WowItemStatic>>[] = await Promise.allSettled(newIds.map((id: number) => {
                return axios.get(staticClient.getWowItemDataUrl(this._patch, id))
            }))

            for (let d of data) {
                if (d.status === 'rejected') {
                    continue
                }
                this._items.set(d.value.data.id, d.value.data)
            }
        }

        itemIds.forEach((id: number) => {
            ret.set(id, this._items.get(id)!)
        })
        return ret
    }
}

export interface WowContentDatum {
    id: number
    name: string
    expansion: string
    parent: number | null
}

class WowPatchStaticCache {
    _patch: Map<string, WowStaticCache> = new Map()

    _cachedArenas: Map<WowGameRelease, WowContentDatum[]> = new Map()
    _cachedDungeons: Map<WowGameRelease, WowContentDatum[]> = new Map()
    _cachedEncounters: Map<WowGameRelease, WowContentDatum[]> = new Map()
    _cachedRaids: Map<WowGameRelease, WowContentDatum[]> = new Map()

    getCache(v: string) {
        let patch = getSupportedWowPatch(v)
        if (!this._patch.has(patch)) {
            this._patch.set(patch, new WowStaticCache(patch))
        }

        return this._patch.get(patch)!
    }

    async getArenas(r: WowGameRelease): Promise<WowContentDatum[]> {
        if (!this._cachedArenas.has(r)) {
            let data: ApiData<WowContentDatum[]> = await axios.get(staticClient.getWowContentArenaUrl(r))
            this._cachedArenas.set(r, data.data)
        }

        return this._cachedArenas.get(r)!
    }

    async getDungeons(r: WowGameRelease): Promise<WowContentDatum[]> {
        if (!this._cachedDungeons.has(r)) {
            let data: ApiData<WowContentDatum[]> = await axios.get(staticClient.getWowContentDungeonsUrl(r))
            this._cachedDungeons.set(r, data.data)
        }

        return this._cachedDungeons.get(r)!
    }

    async getEncounters(r: WowGameRelease): Promise<WowContentDatum[]> {
        if (!this._cachedEncounters.has(r)) {
            let data: ApiData<WowContentDatum[]> = await axios.get(staticClient.getWowContentEncountersUrl(r))
            this._cachedEncounters.set(r, data.data)
        }

        return this._cachedEncounters.get(r)!
    }

    async getRaids(r: WowGameRelease): Promise<WowContentDatum[]> {
        if (!this._cachedRaids.has(r)) {
            let data: ApiData<WowContentDatum[]> = await axios.get(staticClient.getWowContentRaidsUrl(r))
            this._cachedRaids.set(r, data.data)
        }

        return this._cachedRaids.get(r)!
    }
}

export let wowCache = new WowPatchStaticCache()