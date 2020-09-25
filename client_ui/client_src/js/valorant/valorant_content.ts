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

export class ValorantContent {
    _agents : AgentMap

    constructor(data : any) {
        let jData = JSON.parse(data)
        this._agents = {}
        for (let agent of jData["Characters"]) {
            this._agents[agent["ID"]] = agent["Name"] 
        }
    }

    agentIdToName(id : string) : string {
        return this._agents[id.toUpperCase()]
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