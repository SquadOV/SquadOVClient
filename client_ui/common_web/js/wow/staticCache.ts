import axios from 'axios'
import { staticClient } from '@client/js/staticData'
import { ApiData } from '@client/js/api'

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

    constructor() {
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
        
        let data: ApiData<WowBasicStatic> = await axios.get(staticClient.getWowCovenantDataUrl(covenantId))
        this._covenants.set(covenantId, data.data.name)
        return data.data.name
    }

    async getSoulbind(soulbindId: number): Promise<string> {
        if (this._soulbinds.has(soulbindId)) {
            return this._soulbinds.get(soulbindId)!
        }
        
        let data: ApiData<WowBasicStatic> = await axios.get(staticClient.getWowSoulbindDataUrl(soulbindId))
        this._soulbinds.set(soulbindId, data.data.name)
        return data.data.name
    }

    async getConduitItem(conduitId: number): Promise<number> {
        if (this._conduits.has(conduitId)) {
            return this._conduits.get(conduitId)!
        }
        
        let data: ApiData<WowConduitStatic> = await axios.get(staticClient.getWowConduitDataUrl(conduitId))
        this._conduits.set(conduitId, data.data.itemId)
        return data.data.itemId
    }
    
    async getTalent(talentId: number): Promise<WowTalentStatic> {
        if (this._talents.has(talentId)) {
            return this._talents.get(talentId)!
        }

        let data: ApiData<WowTalentStatic> = await axios.get(staticClient.getWowTalentDataUrl(talentId))
        this._talents.set(talentId, data.data)
        return data.data
    }

    async getClassSpec(specId: number): Promise<WowClassSpecStatic> {
        if (this._specs.has(specId)) {
            return this._specs.get(specId)!
        }

        let data: ApiData<WowClassSpecStatic> = await axios.get(staticClient.getWowSpecsDataUrl(specId))
        this._specs.set(specId, data.data)
        return data.data
    }

    async getClass(classId: number): Promise<WowClassStatic> {
        if (this._classes.has(classId)) {
            return this._classes.get(classId)!
        }

        let data: ApiData<WowClassStatic> = await axios.get(staticClient.getWowClassDataUrl(classId))
        this._classes.set(classId, data.data)
        return data.data
    }

    async bulkGetSpellNames(spellIds: number[]): Promise<Map<number, string>> {
        let ret: Map<number, string> = new Map()

        let newIds = spellIds.filter((id: number) => !this._spellNames.has(id))
        if (newIds.length > 0) {
            let data: ApiData<WowBasicStatic>[] = await Promise.all(newIds.map((id: number) => {
                return axios.get(staticClient.getWowSpellDataUrl(id))
            }))

            for (let d of data) {
                this._spellNames.set(parseInt(d.data.id), d.data.name)
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
            let data: ApiData<WowItemStatic>[] = await Promise.all(newIds.map((id: number) => {
                return axios.get(staticClient.getWowItemDataUrl(id))
            }))

            for (let d of data) {
                this._items.set(d.data.id, d.data)
            }
        }

        itemIds.forEach((id: number) => {
            ret.set(id, this._items.get(id)!)
        })
        return ret
    }
}

export let wowCache = new WowStaticCache()