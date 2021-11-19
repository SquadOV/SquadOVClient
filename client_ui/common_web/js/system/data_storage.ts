///#if DESKTOP
import fs from 'fs'
import path from 'path'
///#endif

import VueRouter from 'vue-router'
import {Md5} from 'ts-md5/dist/md5'

export enum DataStorageLocation {
    RecentMatch,
    ClipLibrary,
    Aimlab,
    Csgo,
    Hearthstone,
    Lol,
    Tft,
    Valorant,
    WowArena,
    WowEncounter,
    WowInstance,
    WowKeystone,
}

class DataStorageInterface {
    _router: VueRouter
    constructor(router: VueRouter) {
        this._router = router
    }

    getPathToLocFile(loc: DataStorageLocation): string {
///#if DESKTOP
        let parentFolder = path.join(process.env.SQUADOV_USER_APP_FOLDER!, 'Data')
        if (!fs.existsSync(parentFolder)) {
            fs.mkdirSync(parentFolder)
        }

        let route = Md5.hashStr(this._router.currentRoute.fullPath)
        return path.join(parentFolder, `${loc}-${route}.json`)
///#else
        return ''
///#endif
    }

    getData(loc: DataStorageLocation): any {
///#if DESKTOP
        let fname = this.getPathToLocFile(loc)
        if (fs.existsSync(fname)) {
            try {
                let data = fs.readFileSync(fname , 'utf8')
                return JSON.parse(data)
            } catch (ex) {
                console.log(`Failed to get data from data storage: ${loc} - ${fname}`)
                return null
            }
        } else {
            return null
        }
///#else
        return null
///#endif
    }

    setData(loc: DataStorageLocation, data: any) {
///#if DESKTOP
        let fname = this.getPathToLocFile(loc)
        let tmpFname = `${fname}.tmp`

        fs.writeFileSync(tmpFname, JSON.stringify(data, null, 4), {
            encoding: 'utf-8',
        })

        fs.renameSync(tmpFname, fname)
///#endif
    }
}

let storageSingleton: DataStorageInterface | null  = null

export function initializeDataStorageSingleton(r: VueRouter) {
    storageSingleton = new DataStorageInterface(r)
}

export function getDataStorageSingleton(): DataStorageInterface | null {
    return storageSingleton
}