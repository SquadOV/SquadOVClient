import { TouchBarOtherItemsProxy } from 'electron'
import fs from 'fs'

interface ContentFileMap {
    [patchId : string] : { static: string, abilities: string};
}

const contentFiles : ContentFileMap = {
    'release-01.08-shipping-10-471230': {
        static: 'assets/valorant/content/v1.08.json',    
        abilities: 'assets/valorant/content/v1.08.Abilities.json'
    }
}
const latestPatch = 'release-01.08-shipping-10-471230'

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

        for (let agent of jData["Characters"]) {
            this._agents[agent["ID"]] = agent["Name"] 
        }

        for (let m of jData["Maps"]) {
            const mapName = m["Name"]
            this._mapAssetNameToMap[m["AssetName"]] = mapName
            this._mapAssetPathToMap[m["AssetPath"]] = mapName
        }

        for (let m of jData["GameModes"]) {
            this._gameModes[m["AssetPath"]] = m["Name"] 
        }

        for (let m of jData["Equips"]) {
            this._weapons[m["ID"]] = m["Name"]
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

function loadValorantContent(patchId : string) {
    let usablePatch = patchId
    if (!(patchId in contentFiles)) {
        usablePatch = latestPatch
    }

    let data = fs.readFileSync(contentFiles[usablePatch].static)
    let abilityData = fs.readFileSync(contentFiles[usablePatch].abilities)
    let ct = new ValorantContent(data, abilityData)
    content[patchId] = ct
}

export function getValorantContent(patchId : string) {
    if (!(patchId in content)) {
        loadValorantContent(patchId)
    }

    return content[patchId]
}