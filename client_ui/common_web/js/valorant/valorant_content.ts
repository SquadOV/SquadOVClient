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

    constructor(data : any, abilityData : any) {
        let jData = JSON.parse(data)
        this._agents = {}

        this._mapAssetNameToMap = {}
        this._mapAssetPathToMap = {}
        this._gameModes = {}
        this._weapons = {}

        // All the || here are just meant to support older patches (namely v1.08).
        for (let agent of (jData["Characters"] || jData["characters"])) {
            this._agents[agent["ID"] || agent["id"]] = agent["Name"] || agent["name"]
        }

        for (let m of (jData["Maps"] || jData["maps"])) {
            const mapName = m["Name"] || m["name"]
            this._mapAssetNameToMap[m["AssetName"] || m["assetName"]] = mapName
            this._mapAssetPathToMap[m["AssetPath"] || m["assetPath"]] = mapName
        }

        for (let m of (jData["GameModes"] || jData["gameModes"])) {
            this._gameModes[m["AssetPath"] || m["assetPath"]] = m["Name"] || m["name"]
        }

        for (let m of (jData["Equips"] || jData["equips"])) {
            this._weapons[m["ID"] || m["id"]] = m["Name"] || m["name"]
        }

        this._agentAbilities = JSON.parse(abilityData)
    }

    agentIdToName(id : string) : string {
        return this._agents[id.toUpperCase()]
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

import data from '@client/js/valorant/data/content'
import abilityData from '@client/js/valorant/data/abilities'

function loadValorantContent(patchId : string) {
    let ct = new ValorantContent(data, abilityData)
    content[patchId] = ct
}

export function getValorantContent(patchId : string | null) {
    patchId = 'latest'
    if (!(patchId in content)) {
        loadValorantContent(patchId)
    }

    return content[patchId]
}