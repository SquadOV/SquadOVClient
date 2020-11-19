import { TouchBarOtherItemsProxy } from 'electron'
import fs from 'fs'
const path = require('path')

interface ContentFileMap {
    [patchId : string] : { static: string, abilities: string};
}

const contentFiles : ContentFileMap = {
    'release-01.08': {
        static: process.env.NODE_ENV === 'production' ?  
            path.join(process.resourcesPath, 'assets/valorant/content/v1.08.json') :
            'assets/valorant/content/v1.08.json',
        abilities: process.env.NODE_ENV === 'production' ?  
            path.join(process.resourcesPath, 'assets/valorant/content/v1.08.Abilities.json') :
            'assets/valorant/content/v1.08.Abilities.json'
    },
    'release-01.11': {
        static: process.env.NODE_ENV === 'production' ?  
            path.join(process.resourcesPath, 'assets/valorant/content/v1.11.json') :
            'assets/valorant/content/v1.11.json',
        abilities: process.env.NODE_ENV === 'production' ?  
            path.join(process.resourcesPath, 'assets/valorant/content/v1.11.Abilities.json') :
            'assets/valorant/content/v1.11.Abilities.json'    
    },
    'release-01.12': {
        static: process.env.NODE_ENV === 'production' ?  
            path.join(process.resourcesPath, 'assets/valorant/content/v1.12.json') :
            'assets/valorant/content/v1.12.json',
        abilities: process.env.NODE_ENV === 'production' ?  
            path.join(process.resourcesPath, 'assets/valorant/content/v1.11.Abilities.json') :
            'assets/valorant/content/v1.11.Abilities.json'    
    }
}
const latestPatch = 'release-01.11'

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

const valorantPatchIdRegex = /(release-\d\d.\d\d)-?.*/

export function getValorantContent(patchId : string | null) {
    if (!patchId) {
        patchId = latestPatch
    } else {
        // The patch ID is generally in the form of release-XX.YY-OTHER
        // We just want to extract the release-XX.YY part of the patch.
        let matches = patchId.match(valorantPatchIdRegex)
        if (!matches) {
            patchId = latestPatch
        } else {
            patchId = matches[1]
        }
    }

    if (!(patchId in content)) {
        loadValorantContent(patchId)
    }

    return content[patchId]
}