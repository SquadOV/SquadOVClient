import fs from 'fs'

interface ContentFileMap {
    [patchId : string] : string;
}

const contentFiles : ContentFileMap = {
    'release-01.08-shipping-10-471230': 'assets/valorant/content/v1.08.json',    
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

export class ValorantContent {
    _agents : AgentMap
    _mapAssetNameToMap : GameMapMap
    _mapAssetPathToMap : GameMapMap
    _gameModes : GameModeMap 

    constructor(data : any) {
        let jData = JSON.parse(data)
        this._agents = {}

        this._mapAssetNameToMap = {}
        this._mapAssetPathToMap = {}

        this._gameModes = {}

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

    let data = fs.readFileSync(contentFiles[usablePatch])
    let ct = new ValorantContent(data)
    content[patchId] = ct
}

export function getValorantContent(patchId : string) {
    if (!(patchId in content)) {
        loadValorantContent(patchId)
    }

    return content[patchId]
}