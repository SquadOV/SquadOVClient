import 'regenerator-runtime/runtime.js';
import '@mdi/font/css/materialdesignicons.css' 

import Vue from 'vue'
import Vuetify, {
    VApp,
    VMain,
} from 'vuetify/lib'
import VueRouter from 'vue-router'
import Vuex from 'vuex'

Vue.use(Vuetify)
Vue.use(VueRouter)
Vue.use(Vuex)

/// #if DESKTOP

import App from '@client/vue/App.vue'

/// #else

import WebApp from '@client/vue/WebApp.vue'

/// #endif

const Dashboard = () => import('@client/vue/Dashboard.vue')
const UserProfile = () => import('@client/vue/profile/UserProfile.vue')
const UserSquads = () => import('@client/vue/squads/UserSquads.vue')
const SingleSquadPage = () => import('@client/vue/squads/SingleSquadPage.vue')

const GameLog = () => import('@client/vue/GameLog.vue')
const LogGameChooser = () => import('@client/vue/log/LogGameChooser.vue')
const ValorantLogContainer = () => import('@client/vue/log/ValorantLogContainer.vue')
const ValorantGameLog = () => import('@client/vue/log/ValorantGameLog.vue')
const ValorantMatch = () => import('@client/vue/log/ValorantMatch.vue')

const AimlabGameLog = () => import('@client/vue/log/AimlabGameLog.vue')
const AimlabMatch = () => import('@client/vue/log/AimlabMatch.vue')

const HearthstoneGameLog = () => import('@client/vue/log/HearthstoneGameLog.vue')
const HearthstoneMatch = () => import('@client/vue/log/HearthstoneMatch.vue')
const HearthstoneAllMatchesGameLog = () => import('@client/vue/log/HearthstoneAllMatchesGameLog.vue')
const HearthstoneArenaGameLog = () => import('@client/vue/log/HearthstoneArenaGameLog.vue')
const HearthstoneDuelGameLog = () => import('@client/vue/log/HearthstoneDuelGameLog.vue')
const HearthstoneArenaRunMatchLog = () => import('@client/vue/log/HearthstoneArenaRunMatchLog.vue')
const HearthstoneDuelRunMatchLog = () => import('@client/vue/log/HearthstoneDuelRunMatchLog.vue')
import { HearthstoneGameType } from '@client/js/hearthstone/hearthstone_match'

const WowLogContainer = () => import('@client/vue/log/WowLogContainer.vue')
const WowGameLog = () => import('@client/vue/log/WowGameLog.vue')
const WowMatch = () => import('@client/vue/log/WowMatch.vue')
const WowEncounterGameLog = () => import('@client/vue/utility/wow/WowEncounterGameLog.vue')
const WowKeystoneGameLog = () => import('@client/vue/utility/wow/WowKeystoneGameLog.vue')
const WowDefaultGameLog = () => import('@client/vue/utility/wow/WowDefaultGameLog.vue')

const TftLogContainer = () => import('@client/vue/log/TftLogContainer.vue')
const TftGameLog = () => import('@client/vue/log/TftGameLog.vue')
const TftMatch = () => import('@client/vue/log/TftMatch.vue')

const LolLogContainer = () => import('@client/vue/log/LolLogContainer.vue')
const LolGameLog = () => import('@client/vue/log/LolGameLog.vue')
const LolMatch = () => import('@client/vue/log/LolMatch.vue')

const Performance = () => import('@client/vue/Performance.vue')
const PerformanceComponentChooser = () => import('@client/vue/performance/PerformanceComponentChooser.vue')
const VizStats = () => import('@client/vue/performance/VizStats.vue')

const Login = () => import('@client/vue/auth/Login.vue')
const Register = () => import('@client/vue/auth/Register.vue')

// Shitty names here are a bit of a legacy from merging two different apps together...
// EmailVerify = wait for email verification
// VerifyEmail = endpoint that user goes to when they verify their email.
// LOL.
const EmailVerify = () => import('@client/vue/auth/EmailVerify.vue')
const VerifyEmail = () => import('@client/vue/auth/VerifyEmail.vue')
const ForgotPassword = () => import('@client/vue/auth/ForgotPassword.vue')

const RsoOauthHandler = () => import('@client/vue/auth/oauth/RsoOauthHandler.vue')

import * as pi from '@client/js/pages'

/// #if DESKTOP
import { ipcRenderer } from 'electron'
/// #endif

import { apiClient, ApiData } from '@client/js/api'
import { getSquadOVUser, SquadOVUser } from '@client/js/squadov/user'
import { RootStoreOptions } from '@client/js/vuex/store'

const baseRoutes : any[] = [
    { path: '/', name: pi.DashboardPageId, component: Dashboard },
    { 
        path: '/login',
        name: pi.LoginPageId,
        component: Login,
        props: (route : any) => ({
            reg: route.query.reg
        })
    },
    { path: '/register', name: pi.RegisterPageId, component: Register },
    {
        path: '/forgotpw/:changePasswordId',
        name: pi.ForgotPasswordPageId,
        component: ForgotPassword,
        props: (route : any) => ({
            changePasswordId: route.params.changePasswordId
        })
    },
    {
        path: '/verify',
        name: pi.WaitForVerifyPageId,
        component: EmailVerify,
    },
    { 
        path: '/verify/:verificationId',
        name: pi.VerifyEmailPageId,
        component: VerifyEmail,
        props: (route : any) => ({
            verificationId: route.params.verificationId
        })
    },
    {
        path: '/profile/:userId',
        name: pi.UserProfilePageId,
        component: UserProfile,
        props: (route : any) => ({
            userId: parseInt(route.params.userId)
        })
    },
    {
        path: '/profile/:userId/squads',
        name: pi.UserSquadsPageId,
        component: UserSquads,
        props: (route: any) => ({
            userId: parseInt(route.params.userId)
        })
    },
    {
        path: '/squad/:squadId',
        name: pi.SingleSquadPageId,
        component: SingleSquadPage,
        props: (route: any) => ({
            squadId: parseInt(route.params.squadId)
        })
    },
    { 
        path: '/logs/:userId',
        component: GameLog,
        props: (route: any) => ({
            squadId: parseInt(route.query.squadId),
            userId: parseInt(route.params.userId),
        }),
        children: [
            {
                path: '',
                name: pi.LogPageId,
                component: LogGameChooser
            },
            {
                path: 'valorant/account/:account?',
                children: [
                    {
                        path: '',
                        name: pi.ValorantLogPageId,
                        component: ValorantGameLog,
                        props: (route : any) => ({
                            userId: parseInt(route.params.userId),
                            puuid: route.params.account
                        })
                    },
                ],
                component: ValorantLogContainer,
                props: (route : any) => ({
                    userId: parseInt(route.params.userId),
                    puuid: route.params.account
                })
            },
            {
                path: 'tft/account/:account?',
                children: [
                    {
                        path: '',
                        name: pi.TftLogPageId,
                        component: TftGameLog,
                        props: (route : any) => ({
                            userId: parseInt(route.params.userId),
                            puuid: route.params.account
                        })
                    },
                ],
                component: TftLogContainer,
                props: (route : any) => ({
                    userId: parseInt(route.params.userId),
                    puuid: route.params.account
                })
            },
            {
                path: 'lol/account/:account?',
                children: [
                    {
                        path: '',
                        name: pi.LolLogPageId,
                        component: LolGameLog,
                        props: (route : any) => ({
                            userId: parseInt(route.params.userId),
                            puuid: route.params.account
                        })
                    },
                ],
                component: LolLogContainer,
                props: (route : any) => ({
                    userId: parseInt(route.params.userId),
                    puuid: route.params.account
                })
            },
            {
                path: 'aimlab',
                name: pi.AimlabLogPageId,
                component: AimlabGameLog,
                props: (route : any) => ({
                    userId: parseInt(route.params.userId),
                })
            },
            {
                path: 'hearthstone',
                component: HearthstoneGameLog,
                props: (route : any) => ({
                    userId: parseInt(route.params.userId),
                }),
                children: [
                    {
                        path: '',
                        name: pi.HearthstoneLogPageId,
                        component: HearthstoneAllMatchesGameLog,
                        props: (route : any) => ({
                            userId: parseInt(route.params.userId),
                        }),
                    },
                    {
                        path: 'constructed',
                        name: pi.HearthstoneConstructedLogPageId,
                        component: HearthstoneAllMatchesGameLog,
                        props: (route : any) => ({
                            filteredGameTypes: [
                                HearthstoneGameType.Ranked,
                                HearthstoneGameType.Casual
                            ],
                            userId: parseInt(route.params.userId),
                        })
                    },
                    {
                        path: 'arena',
                        name: pi.HearthstoneArenaLogPageId,
                        component: HearthstoneArenaGameLog,
                        props: (route : any) => ({
                            userId: parseInt(route.params.userId),
                        }),
                    },
                    {
                        path: 'arena/:arenaId',
                        name: pi.HearthstoneArenaRunMatchLogPageId,
                        component: HearthstoneArenaRunMatchLog,
                        props: (route: any) => ({
                            runId: route.params.arenaId,
                            userId: parseInt(route.params.userId),
                        })
                    },
                    {
                        path: 'brawl',
                        name: pi.HearthstoneBrawlLogPageId,
                        component: HearthstoneAllMatchesGameLog,
                        props: (route : any) => ({
                            filteredGameTypes: [
                                HearthstoneGameType.TavernBrawl,
                                HearthstoneGameType.Tb1pVsAi,
                                HearthstoneGameType.Tb2pCoop,
                                HearthstoneGameType.FsgBrawlVsFriend,
                                HearthstoneGameType.FsgBrawl,
                                HearthstoneGameType.FsgBrawl1pVsAi,
                                HearthstoneGameType.FsgBrawl2pCoop,
                            ],
                            userId: parseInt(route.params.userId),
                        })
                    },
                    {
                        path: 'battlegrounds',
                        name: pi.HearthstoneBattlegroundsLogPageId,
                        component: HearthstoneAllMatchesGameLog,
                        props: (route : any) => ({
                            filteredGameTypes: [
                                HearthstoneGameType.Battlegrounds,
                                HearthstoneGameType.BattlegroundsFriendly
                            ],
                            userId: parseInt(route.params.userId),
                        })
                    },
                    {
                        path: 'duels',
                        name: pi.HearthstoneDuelLogPageId,
                        component: HearthstoneDuelGameLog,
                        props: (route : any) => ({
                            userId: parseInt(route.params.userId),
                        }),
                    },
                    {
                        path: 'duels/:duelId',
                        name: pi.HearthstoneDuelRunMatchLogPageId,
                        component: HearthstoneDuelRunMatchLog,
                        props: (route: any) => ({
                            runId: route.params.duelId,
                            userId: parseInt(route.params.userId),
                        })
                    },
                ]
            },
            {
                path: 'wow/character/:guid?',
                children: [
                    {
                        path: '',
                        component: WowGameLog,
                        props: (route : any) => ({
                            userId: parseInt(route.params.userId),
                            guid: route.params.guid,
                        }),
                        children: [
                            {
                                path: '',
                                name: pi.WowLogPageId,
                                // Use this as the component instead of using a redirect
                                // so that when the game nav tries to get the path of this route
                                // we get the non-redirected URL.
                                component: WowDefaultGameLog,
                            },
                            {
                                path: 'encounters',
                                name: pi.WowEncounterLogPageId,
                                component: WowEncounterGameLog,
                                props: (route : any) => ({
                                    userId: parseInt(route.params.userId),
                                    guid: route.params.guid,
                                }),
                            },
                            {
                                path: 'challenges',
                                name: pi.WowChallengeLogPageId,
                                component: WowKeystoneGameLog,
                                props: (route : any) => ({
                                    userId: parseInt(route.params.userId),
                                    guid: route.params.guid,
                                }),
                            }
                        ]
                    },
                ],
                component: WowLogContainer,
                props: (route : any) => ({
                    userId: parseInt(route.params.userId),
                    guid: route.params.guid,
                }),
            }
        ],
    },
    {
        path: '/match/valorant/:matchUuid',
        name: pi.ValorantMatchPageId,
        component: ValorantMatch,
        props: (route : any) => ({
            puuid: route.query.account,
            matchUuid: route.params.matchUuid,
            userId: parseInt(route.query.userId)
        })
    },
    {
        path: '/match/tft/:matchUuid',
        name: pi.TftMatchPageId,
        component: TftMatch,
        props: (route : any) => ({
            puuid: route.query.account,
            matchUuid: route.params.matchUuid,
            userId: parseInt(route.query.userId)
        })
    },
    {
        path: '/match/lol/:matchUuid',
        name: pi.LolMatchPageId,
        component: LolMatch,
        props: (route : any) => ({
            puuid: route.query.account,
            matchUuid: route.params.matchUuid,
            userId: parseInt(route.query.userId)
        })
    },
    {
        path: '/match/aimlab/:taskId',
        name: pi.AimlabMatchPageId,
        component: AimlabMatch,
        props: (route : any) => ({
            taskId: route.params.taskId,
            userId: parseInt(route.query.userId)
        })
    },
    {
        path: '/match/hearthstone/:matchId',
        name: pi.HearthstoneMatchPageId,
        component: HearthstoneMatch,
        props: (route : any) => ({
            matchId: route.params.matchId,
            userId: parseInt(route.query.userId)
        })
    },
    {
        path: '/match/wow/:matchUuid',
        name: pi.WowMatchPageId,
        component: WowMatch,
        props: (route : any) => ({
            userId: parseInt(route.query.userId),
            matchUuid: route.params.matchUuid
        }),
    },
    { 
        path: '/performance',
        component: Performance,
        children: [
            {
                path: '',
                name: pi.PerformancePageId,
                component: PerformanceComponentChooser,
            },
            {
                path: 'visualization',
                name: pi.VizStatsPageId,
                component: VizStats,
            },
        ]
    },
    {
        path: '/riot/oauth-callback',
        component: RsoOauthHandler,
        name: pi.RsoOauthPageId,
        props: (route : any) => ({
            code: route.query.code,
            state: route.query.state,
        }),
    }
]

const router = new VueRouter({
/// #if !DESKTOP
    mode :'history',
/// #endif
    routes: baseRoutes,
})

const store = new Vuex.Store(RootStoreOptions)
import { loadInitialSessionFromCookies, checkHasSessionCookie } from '@client/js/session'

router.beforeEach((to : any, from : any, next : any) => {
    console.log(`Navigate ${from.fullPath} (${from.name}) => ${to.fullPath} (${to.name})`)
    
    let mustBeInvalid = (to.name === pi.LoginPageId || to.name === pi.RegisterPageId || to.name === pi.ForgotPasswordPageId)
    if (to.name != pi.DashboardPageId && !mustBeInvalid) {
        apiClient.markUserActive().catch((err: any) => {
            console.log('Failed to mark user active: ', err)
        })
    }

    // Allow OAuth requests to always go through regardless of whether the user is signed in or not.
    if (to.name === pi.RsoOauthPageId) {
        next()
        return
    }

/// #if DESKTOP
    next()
/// #else
    let hasCookie = checkHasSessionCookie()
    if (hasCookie && !store.state.currentUser) {
        // If we have a cookie and the current user isn't set, then send out a request
        // to verify the session cookie.
        loadInitialSessionFromCookies(store, () => {
            // Note that in the web case, the renderer code is responsible for the doing the session
            // heartbeat as well.
            if (mustBeInvalid) {
                next({
                    name: pi.DashboardPageId
                })
            } else {
                next()
            }
        }, () => {
            if (!mustBeInvalid) {
                next({
                    name: pi.LoginPageId
                })
            } else {
                next()
            }
        })
    } else {
        if (mustBeInvalid && hasCookie) {
            next({
                name: pi.DashboardPageId
            })
        } else if (!mustBeInvalid && !hasCookie) {
            next({
                name: pi.LoginPageId
            })
        } else {
            next()
        }
    }
/// #endif
})

/// #if DESKTOP
store.dispatch('reloadLocalSettings')

ipcRenderer.on('reset-state', () => {
    store.commit('resetState')
})

// As soon as the app starts we need to query the main process for
// the session ID to properly initialize the API client
// so that it's authenticated with the web server. After that, send out
// an immediate request to obtain the current user profile and store it in the
// Vuex storage to make it available to the frontend.
ipcRenderer.invoke('request-session').then((session : {
    sessionId: string,
    userId: string
}) => {
    apiClient.setSessionId(session.sessionId)
    getSquadOVUser(parseInt(session.userId)).then((resp : ApiData<SquadOVUser>) => {
        store.commit('setUser' , resp.data)
    }).catch((err : any ) => {
        // Uhhhhhhhhhhhhhhhhhhhhhhh....? Need to logout here since
        // the stored session is garbage and so we have no way to recover
        // unless the user re-logs in. There's a chance that the API might have failed
        // so don't bother trying to do a proper API logout.
        console.log('Failed to obtain user: ', err)
        ipcRenderer.sendSync('logout')
    })
})

let globals: any = {
    serviceError: false,
    hasUpdate: false,
}

ipcRenderer.on('service-error', () => {
    globals.serviceError = true
})

ipcRenderer.on('main-update-downloaded', () => {
    globals.hasUpdate = true
})

ipcRenderer.on('change-running-games', (_, games) => {
    store.commit('setRunningGames', games)
})

ipcRenderer.on('change-recording-games', (_, games) => {
    store.commit('setRecordingGames', games)
})

/// #endif

new Vue({
    el: '#app',
    components: {
        VApp,
        VMain,
/// #if DESKTOP
        App,
/// #else
        WebApp,
/// #endif
    },
    vuetify: new Vuetify({
        theme: {
            dark: true,
        },
    }),
    data: () => ({
/// #if DESKTOP
        globals
/// #endif
    }),
    methods: {
        generateAssetUri(s: string): string {
/// #if DESKTOP
            return s
/// #else
            return `/${s}`
/// #endif
        }
    },
    router: router,
    store: store,
}).$mount('#app')

/// #if DESKTOP
console.log('Loading Vue Application - Desktop')
/// #else
console.log('Loading Vue Application - Web')
/// #endif