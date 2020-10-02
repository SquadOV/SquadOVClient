import StatLibrary from '@client/js/stats/stat_library'

function splitStatsOnDepthFolder(stats : string[], level: number) : Map<string, string[]> {
    let statPaths = stats.map((ele : string) => StatLibrary.getStatName(ele)!.split('/'))
    let validPaths = statPaths.map((ele : string[]) => ele.length > level)
    let statFolders = stats.map((ele : string, idx : number) => {
        if (!validPaths[idx]) {
            return null
        }
        return statPaths[idx][level]
    })

    let retMap = new Map<string, string[]>()
    for (let i = 0; i < stats.length; ++i) {
        let key = statFolders[i]
        if (!key) {
            continue
        }

        if (!retMap.has(key)) {
            retMap.set(key, [])
        }

        let val = retMap.get(key)!
        val.push(stats[i])
        retMap.set(key, val)
    }
    return retMap
}

// Build a tree of stats so that we can present it in a semi-organized human usable form.
// So the stat aimlab_gridshot_ultimate_score would be shown as
// Aimlab / Gridshot / Ultimate / Score where there'd be 3 folders and 1 leaf stat.
export class StatTreeNode {
    node: string
    parent: string
    level: number
    childStats: string[]
    childNodes?: StatTreeNode[]
    stat? : string

    get currentPath() : string {
        return `${this.parent}/${this.node}`
    }

    constructor(node: string, stats: string[], level: number, parent : string = '') {
        this.node = node
        this.parent = parent
        this.level = level
        this.childStats = stats
     
        let subStats = splitStatsOnDepthFolder(stats, level + 1)
        if (subStats.size > 0) {
            let subFolders = Array.from(subStats.keys()).sort()
            this.childNodes = subFolders.map((ele : string) => new StatTreeNode(ele, subStats.get(ele)!, level + 1, this.currentPath))
        } else {
            this.stat = stats[0]
        }
    }
}

export function buildVuetifyTreeViewItemFromStatTreeNode(n : StatTreeNode) : any {
    if (!!n.stat) {
        return {
            id: n.stat,
            name: n.node,
        }
    } else {
        return {
            id: n.currentPath,
            name: n.node,
            children: n.childNodes!.map(buildVuetifyTreeViewItemFromStatTreeNode),
        }
    }
}

export class StatTree {
    rootNodes: StatTreeNode[]

    constructor(stats: string[]) {
        let topLevelStats = splitStatsOnDepthFolder(stats, 0)
        let folders = Array.from(topLevelStats.keys()).sort()
        this.rootNodes = folders.map((ele : string) => new StatTreeNode(ele, topLevelStats.get(ele)!, 0))
    }
}