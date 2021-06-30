interface AgentMap {
    [agentId : string] : string;
}

interface GameMapMap {
    [mapId : string] : string;
}

interface GameModeMap {
    [modeId : string] : string;
}

export type AbilityKey = "GrenadeAbility" | "Ability1" | "Ability2" | "Ultimate"
export class ValorantContent {
    _agents : AgentMap
    _agentAbilities : { [ agentId : string] : {
        [T in AbilityKey] : string   
    }}
    _mapAssetNameToMap : GameMapMap
    _mapAssetPathToMap : GameMapMap
    _gameModes : GameModeMap 
    _weapons : { [weaponId : string] : string }

    availableMaps: string[]
    availableModes: string[]

    constructor(data : any, abilityData : any) {
        let jData = data
        this._agents = {}

        this._mapAssetNameToMap = {}
        this._mapAssetPathToMap = {}
        this._gameModes = {}
        this._weapons = {}

        this.availableMaps = []
        this.availableModes = []

        // All the || here are just meant to support older patches (namely v1.08).
        for (let agent of (jData["Characters"] || jData["characters"])) {
            this._agents[agent["ID"] || agent["id"]] = agent["Name"] || agent["name"]
        }

        for (let m of (jData["Maps"] || jData["maps"])) {
            const mapName = m["Name"] || m["name"]
            if (mapName == "Null UI Data!") {
                continue
            }
            this._mapAssetNameToMap[m["AssetName"] || m["assetName"]] = mapName

            let path = m["AssetPath"] || m["assetPath"]
            this._mapAssetPathToMap[path] = mapName
            this.availableMaps.push(path)
        }

        for (let m of (jData["GameModes"] || jData["gameModes"])) {
            let name = m["Name"] || m["name"]
            if (name == "Onboarding" || name == "PRACTICE" || name == "SwiftPlay") {
                continue
            }

            let path = m["AssetPath"] || m["assetPath"]
            this._gameModes[path] = name
            this.availableModes.push(path)
        }

        for (let m of (jData["Equips"] || jData["equips"])) {
            this._weapons[m["ID"] || m["id"]] = m["Name"] || m["name"]
        }

        this._agentAbilities = abilityData
    }

    agentIdToName(id : string) : string {
        return this._agents[id.toUpperCase()]
    }

    agentIdToFilesystemName(id : string) : string {
        let name = this.agentIdToName(id)
        if (name == 'KAY/O') {
            return 'Kayo'
        } else {
            return name
        }
    }

    mapAssetNameToName(id: string) : string {
        return this._mapAssetNameToMap[id]
    }

    mapAssetPathToName(id: string) : string {
        return this._mapAssetPathToMap[id]
    }

    gameModeToName(path : string) : string {
        return this._gameModes[path]
    }

    weaponIdToName(id : string) : string{
        return this._weapons[id]
    }

    abilityIdToName(agentId : string, ability : AbilityKey) : string {
        return this._agentAbilities[this.agentIdToName(agentId)][ability]
    }
}

interface ContentMap {
    [patchId : string] : ValorantContent;
}
let content : ContentMap = {}

import data from '@client/js/valorant/data/content.json'
import abilityData from '@client/js/valorant/data/abilities.json'

function loadValorantContent(patchId : string) {
    let ct = new ValorantContent(data, abilityData)
    content[patchId] = ct
}

export function getValorantContent(patchId : string | null): ValorantContent {
    patchId = 'latest'
    if (!(patchId in content)) {
        loadValorantContent(patchId)
    }

    return content[patchId]
}