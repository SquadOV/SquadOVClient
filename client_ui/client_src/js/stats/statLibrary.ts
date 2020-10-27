import { createAimlabStatFolder } from '@client/js/stats/aimlabStatLibrary'
import { StatFolder, StatObject, StatInstance } from '@client/js/stats/statPrimitives'

class StatLibrary {
    _root: StatFolder

    constructor(rootFolder: StatFolder) {
        this._root = rootFolder
    }

    private getAllStatIdsHelper(f : StatFolder, prefix : string) : string[] {
        let ret : string[] = []
        
        if (!!f.childFolders) {
            for (let [id, value] of Object.entries(f.childFolders)) {
                ret.push(...this.getAllStatIdsHelper(value, `${prefix}.${id}`))
            }
        }

        if (!!f.stats) {
            for (let [id, _] of Object.entries(f.stats)) {
                ret.push(`${prefix}.${id}`)
            }
        }
        return ret
    }

    get allStatIds() : string[] {
        return this.getAllStatIdsHelper(this._root, 'root')
    }

    private getVuetifyTreeViewItemsHelper(f : StatFolder, prefix: string)  : any {
        let ret : any = {
            id: prefix,
            name: f.name,
            children: [],
        }

        if (!!f.childFolders) {
            for (let [id, value] of Object.entries(f.childFolders)) {
                ret.children.push(this.getVuetifyTreeViewItemsHelper(value, `${prefix}.${id}`))
            }
        }

        if (!!f.stats) {
            for (let [id, value] of Object.entries(f.stats)) {
                ret.children.push({
                    id: `${prefix}.${id}`,
                    name: value.name,
                })
            }
        }


        ret.children.sort((a : any, b: any) => {
            if (a.name < b.name) {
                return -1
            } else if (a.name >  b.name) {
                return 1
            }
            return 0
        })

        return ret
    }

    get vuetifyTreeViewItems() : any[] {
        return this.getVuetifyTreeViewItemsHelper(this._root, 'root').children
    }

    splitId(id: string) : string[] {
        return id.split('.')
    }

    private getStatNameHelper(f : StatFolder, path: string[], current: string) : string {
        if (path.length == 1) {
            let stat = f.stats![path[0]]
            return `${current}/${stat.name}`
        } else {
            let folder = f.childFolders![path[0]]
            let newCurrent = `${current}/${folder.name}`
            return this.getStatNameHelper(folder, path.slice(1), newCurrent)
        }
    }

    getStatName(path: string[]) : string {
        // Assume that the first element is always 'root'
        return this.getStatNameHelper(this._root, path.slice(1), '')
    }

    private getStatObjectHelper(f : StatFolder, path: string[]) : StatObject {
        if (path.length == 1) {
            return f.stats![path[0]]
        } else {
            let folder = f.childFolders![path[0]]
            return this.getStatObjectHelper(folder, path.slice(1))
        }
    }

    getStatObject(path: string[]) : StatObject {
        return this.getStatObjectHelper(this._root, path.slice(1))
    }

    createInstance(id : string) : StatInstance {
        let path = this.splitId(id)
        let obj = this.getStatObject(path)
        let optionData : any = {}
        for (let opt of obj.availableOptions) {
            optionData[opt.id] = opt.createDefault()
        }

        return {
            id: id,
            name: this.getStatName(path),
            obj: obj,
            options: {
                options: obj.availableOptions,
                data: optionData,
            }
        }
    }
}

export const statLibrary = new StatLibrary({
    name: "Root",
    childFolders: {
        'aimlab': createAimlabStatFolder(),
    }
})