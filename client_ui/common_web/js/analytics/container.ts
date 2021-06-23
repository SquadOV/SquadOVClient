import { Store } from 'vuex'
import { RootState } from '../vuex/state'
import { v4 as uuidv4 } from 'uuid'
import {
    AnalyticsCategory,
    AnalyticsAction
} from '@client/js/analytics/events'
import ua from 'universal-analytics'

///#if DESKTOP
import fs from 'fs'
import path from 'path'
///#else
///#endif

export class AnalyticsContainer {
    _isInit: boolean = false
    _store: Store<RootState>
    _ga: ua.Visitor

    generateOrLoadClientId(): string {
        console.log('Checking for anonymous analytics ID...')
        // Check if one exists already
///#if DESKTOP
        let fname = path.join(process.env.SQUADOV_USER_APP_FOLDER!, '.anonymous')
        if (fs.existsSync(fname)) {
            console.log('...loading from disk', fname)
            return fs.readFileSync(fname, 'utf8')
        }
///#else
        const key = '_sqAnonymousId'
        const id = window.localStorage.getItem(key)
        if (!!id) {
            console.log('...loading from local storage')
            return id
        }
///#endif
        console.log('Generating new anonymous analytics ID...')
        // If we get here, one does not exist, need to create and store
        let newClientId = uuidv4()

///#if DESKTOP
        fs.writeFileSync(fname, newClientId)
///#else
        window.localStorage.setItem(key, newClientId)
///#endif

        return newClientId
    }

    constructor(store: Store<RootState>) {
        this._store = store

        // Generate or load a uuid for this [anonymous] user.
        this._ga = ua('UA-185942570-1', this.generateOrLoadClientId())
///#if DESKTOP
        this._ga.set('ds', 'app')
///#else
        this._ga.set('ds', 'web')
///#endif
        this._ga.set('aip', true)
        // isLoggedIn
        this._ga.set('cd[1]', (!!this._store.state.currentUser).toString())
    }

    pageView(fullPath: string) {
        this._ga.pageview(fullPath).send()
    }

    event(fullPath: string, category: AnalyticsCategory, action: AnalyticsAction, label: string, value: number) {
        let params = {
            ec: AnalyticsCategory[category],
            ea: AnalyticsAction[action],
            el: label,
            dp: fullPath,
            ev: value,
        }

        this._ga.event(params).send()
    }
}

let analytics: AnalyticsContainer | null = null
export function initializeAnalyticsContainer(store: Store<RootState>) {
    if (!!analytics) {
        return
    }

    analytics = new AnalyticsContainer(store)
}

export function getAnalyticsContainer(): AnalyticsContainer {
    return analytics!
}