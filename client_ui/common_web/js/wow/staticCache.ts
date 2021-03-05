import axios from 'axios'
import { staticClient } from '@client/js/staticData'
import { ApiData } from '@client/js/api'

interface WowItem {
    id: string
    name: string
}

class WowStaticCache {
    _spellNames: Map<number, string>

    constructor() {
        this._spellNames = new Map()
    }

    async bulkGetSpellNames(spellIds: number[]): Promise<Map<number, string>> {
        let ret: Map<number, string> = new Map()

        let newIds = spellIds.filter((id: number) => !this._spellNames.has(id))
        if (newIds.length > 0) {
            let data: ApiData<WowItem>[] = await Promise.all(newIds.map((id: number) => {
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
}

export let wowCache = new WowStaticCache()