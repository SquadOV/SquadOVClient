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
import { apiClient } from '../api'
///#else
///#endif

export class AnalyticsContainer {
    _isInit: boolean = false
    _store: Store<RootState>
    _anonId: string
    _analytics: Analytics | null = null
    _identified: boolean = false
    _ip: string | undefined = undefined

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

    get context(): any {
        return {
            app: {
                name: 'SquadOV',
                version: rawData.version,
                build: 'Release'
            },
            ip: this._ip,
        }
    }

    identify(ip: string) {
        if (!this._analytics) {
            return
        }

        this._ip = ip
        this._analytics.identify({
            userId: this._store.state.currentUser?.uuid,
            anonymousId: this._anonId,
            traits: {
                email: this._store.state.currentUser?.email,
                username: this._store.state.currentUser?.username,
            },
            context: this.context,
            integrations: {                                
                'Google Analytics': 
///#if DESKTOP                
                {
                    clientId: this._anonId,
                }
///#else
                false
///#endif
            }
        })

///#if DESKTOP
        if (!!this._store.state.currentUser) {
            apiClient.secondaryIdentify(this._ip, this._anonId).catch((err: any) => {
                console.error('Failed to do secondary identify.')
            })
        }
///#endif
        this._identified = true
    }

    pageView(route: Route) {
        if (!this._analytics ||
///#if DESKTOP
            !this._store.state.settings?.anonymousAnalytics ||
///#endif
            !this._identified) {
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
                'Google Analytics': 
///#if DESKTOP                
                {
                    clientId: this._anonId,
                }
///#else
                false
///#endif
            },
            context: this.context,
        })
    }

    event(route: Route, category: AnalyticsCategory, action: AnalyticsAction, label: string, value: number) {
        if (!this._analytics||
///#if DESKTOP
            !this._store.state.settings?.anonymousAnalytics ||
///#endif
            !this._identified) {
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
                'Google Analytics':
///#if DESKTOP                
                {
                    clientId: this._anonId,
                }
///#else
                false
///#endif
            },
            context: {
                page: {
                    path: route.fullPath,
                    title: !!route.name ? route.name : 'Unknown',
                },
                ...this.context,
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