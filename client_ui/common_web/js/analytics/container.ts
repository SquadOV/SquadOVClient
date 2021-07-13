import { Store } from 'vuex'
import { RootState } from '../vuex/state'
import { v4 as uuidv4 } from 'uuid'
import {
    AnalyticsCategory,
    AnalyticsAction
} from '@client/js/analytics/events'
import ua from 'universal-analytics'
import mixpanel from 'mixpanel-browser'

///#if DESKTOP
import fs from 'fs'
import path from 'path'
import { Route } from 'vue-router'
///#else
///#endif

export class AnalyticsContainer {
    _isInit: boolean = false
    _store: Store<RootState>
    _ga: ua.Visitor | null = null

    get platform(): string {
///#if DESKTOP
        return 'app'
///#else
        return 'web'
///#endif
    }

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

        if (!SQUADOV_USE_ANALYTICS) {
            return
        }

        let anonId = this.generateOrLoadClientId()
        // Generate or load a uuid for this [anonymous] user.
        this._ga = ua('UA-185942570-1', anonId)
        this._ga.set('ds', this.platform)
        this._ga.set('aip', true)
        // isLoggedIn
        this._ga.set('cd[1]', (!!this._store.state.currentUser).toString())

        mixpanel.init('68b7bcde2b8c2ae564daf927e46d391f')
        mixpanel.identify(anonId)
    }

    pageView(route: Route) {
        if (!SQUADOV_USE_ANALYTICS || !this._ga) {
            return
        }

        if (!this._store.state.settings?.anonymousAnalytics) {
            return
        }
        this._ga.pageview({
            dp: route.fullPath,
            dh: 'https://app.squadov.gg',
            dt: !!route.name ? route.name : 'Unknown',
        }).send()
    }

    event(route: Route, category: AnalyticsCategory, action: AnalyticsAction, label: string, value: number) {
        if (!SQUADOV_USE_ANALYTICS) {
            return
        }

        if (!this._store.state.settings?.anonymousAnalytics) {
            return
        }
        
        let routeName = !!route.name ? route.name : 'Unknown'
        let params = {
            ec: AnalyticsCategory[category],
            ea: AnalyticsAction[action],
            el: label,
            dp: route.fullPath,
            dt: routeName,
            ev: value,
        }

        if (!!this._ga) {
            this._ga.event(params).send()
        }
        
        mixpanel.track(`${params.ec}-${params.ea}`, {
            path: route.fullPath,
            route: routeName,
            loggedIn: !!this._store.state.currentUser,
            label,
            value,
            platform: this.platform,
        })
    }
}

let analytics: AnalyticsContainer | null = null
export function initializeAnalyticsContainer(store: Store<RootState>) {
    if (!!analytics) {
        return
    }

    analytics = new AnalyticsContainer(store)
}

export function getAnalyticsContainer(): AnalyticsContainer | null {
    return analytics!
}