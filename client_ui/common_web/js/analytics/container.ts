import { Store } from 'vuex'
import { RootState } from '../vuex/state'
import { v4 as uuidv4 } from 'uuid'
import {
    AnalyticsCategory,
    AnalyticsAction
} from '@client/js/analytics/events'
import Analytics from 'analytics-node'
import rawData from '@client/package.json'

///#if DESKTOP
import fs from 'fs'
import path from 'path'
import { Route } from 'vue-router'
///#else
///#endif

export class AnalyticsContainer {
    _isInit: boolean = false
    _store: Store<RootState>
    _anonId: string
    _analytics: Analytics | null = null
    _identified: boolean = false

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

        // Generate or load a uuid for this [anonymous] user.
        this._anonId = this.generateOrLoadClientId()

        // Initialize Segment
        this._analytics = new Analytics(SQUADOV_SEGMENT_KEY)
    }

    identify() {
        if (!this._analytics) {
            return
        }

        this._analytics.identify({
            userId: this._store.state.currentUser?.uuid,
            anonymousId: this._anonId,
            traits: {
                isLoggedIn: (!!this._store.state.currentUser).toString(),
                email: this._store.state.currentUser?.email,
            },
            context: {
                app: {
                    name: 'SquadOV',
                    version: rawData.version,
                    build: 'Release'
                }
            }
        })
        this._identified = true
    }

    pageView(route: Route) {
        if (!this._analytics || !this._store.state.settings?.anonymousAnalytics || !this._identified) {
            return
        }

        this._analytics.page({
            userId: this._store.state.currentUser?.uuid,
            anonymousId: this._anonId,
            name: !!route.name ? route.name : 'Unknown',
            properties: {
                url: `https://app.squadov.gg${route.fullPath}`,
                path: route.fullPath,
            },
            integrations: {
                All: true,
                Vero: false,
            },
        })
    }

    event(route: Route, category: AnalyticsCategory, action: AnalyticsAction, label: string, value: number) {
        if (!this._analytics || !this._store.state.settings?.anonymousAnalytics || !this._identified) {
            return
        }

        this._analytics.track({
            userId: this._store.state.currentUser?.uuid,
            anonymousId: this._anonId,
            event: AnalyticsAction[action],
            properties: {
                category: AnalyticsCategory[category],
                label,
                value,
            },
            integrations: {
                All: true,
                Vero: false,
            },
            context: {
                page: {
                    path: route.fullPath,
                    title: !!route.name ? route.name : 'Unknown',
                }
            }
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