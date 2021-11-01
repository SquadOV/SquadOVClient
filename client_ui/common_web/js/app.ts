import 'regenerator-runtime/runtime.js';
import '@mdi/font/css/materialdesignicons.css' 

import Vue from 'vue'
import Vuetify, {
    VApp,
    VMain,
} from 'vuetify/lib'
import VueRouter, { Route } from 'vue-router'
import Vuex from 'vuex'
import VueMeta from 'vue-meta'

Vue.use(Vuetify)
Vue.use(VueRouter)
Vue.use(Vuex)
Vue.use(VueMeta)

import App from '@client/vue/App.vue'

/// #if DESKTOP

import PrimaryApp from '@client/vue/PrimaryApp.vue'

/// #else

import WebApp from '@client/vue/WebApp.vue'

/// #endif

const Dashboard = () => import('@client/vue/Dashboard.vue')
const UserProfile = () => import('@client/vue/profile/UserProfile.vue')
const UserSquads = () => import('@client/vue/squads/UserSquads.vue')
const SingleSquadPage = () => import('@client/vue/squads/SingleSquadPage.vue')
const TitleBar = () => import('@client/vue/TitleBar.vue')

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
const WowArenaGameLog = () => import('@client/vue/utility/wow/WowArenaGameLog.vue')
const WowDefaultGameLog = () => import('@client/vue/utility/wow/WowDefaultGameLog.vue')

const TftLogContainer = () => import('@client/vue/log/TftLogContainer.vue')
const TftGameLog = () => import('@client/vue/log/TftGameLog.vue')
const TftMatch = () => import('@client/vue/log/TftMatch.vue')

const LolLogContainer = () => import('@client/vue/log/LolLogContainer.vue')
const LolGameLog = () => import('@client/vue/log/LolGameLog.vue')
const LolMatch = () => import('@client/vue/log/LolMatch.vue')

const CsgoGameLog = () => import('@client/vue/log/CsgoGameLog.vue')
const CsgoMatch = () => import('@client/vue/log/CsgoMatch.vue')

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
const TwitchOauthHandler = () => import('@client/vue/auth/oauth/TwitchOauthHandler.vue')

const SquadInviteResponsePage = () => import('@client/vue/squads/SquadInviteResponsePage.vue')
const SquadLinkResponsePage = () => import('@client/vue/squads/SquadLinkResponsePage.vue')
const ShareRedirect = () => import('@client/vue/ShareRedirect.vue')
const VodEditor = () => import('@client/vue/utility/vods/VodEditor.vue')
const AppSettingsPage = () => import('@client/vue/utility/squadov/AppSettingsPage.vue')

const ClipLibrary = () => import('@client/vue/utility/vods/ClipLibrary.vue')
const ClipView = () => import('@client/vue/utility/vods/ClipView.vue')
const SetupWizard = () => import('@client/vue/SetupWizard.vue')
const Player = () => import('@client/vue/Player.vue')
const LocalStorageManager = () => import ('@client/vue/LocalStorageManager.vue')

const Community = () => import('@client/vue/Community.vue')
const CommunityFeed = () => import('@client/vue/utility/community/CommunityFeed.vue')
const CommunityPage = () => import('@client/vue/utility/community/CommunityPage.vue')
const CommunityDiscover = () => import('@client/vue/utility/community/CommunityDiscover.vue')

const UserFavoritesPage = () => import('@client/vue/profile/UserFavoritesPage.vue')
const UserWatchlistPage = () => import('@client/vue/profile/UserWatchlistPage.vue')
const PostGameReport = () => import ('@client/vue/PostGameReport.vue')

import * as pi from '@client/js/pages'

/// #if DESKTOP
import { ipcRenderer } from 'electron'
/// #endif

import { apiClient, ApiData } from '@client/js/api'
import { getSquadOVUser, SquadOVUser } from '@client/js/squadov/user'
import { RootStoreOptions } from '@client/js/vuex/store'
import { timeStringToSeconds } from '@client/js/time'

function isDesktop(): boolean {
/// #if DESKTOP
    return true
/// #else
    return false
/// #endif
}

const baseRoutes : any[] = [
    {
        path: '/editor/:videoUuid',
        component: VodEditor,
        name: pi.VideoEditorPageId,
        props: (route: any) => ({
            videoUuid: route.params.videoUuid,
            game: parseInt(route.query.game),
        })
    },
    {
        path: '/player/:vodUuid',
        name: pi.PlayerPageId,
        component: Player,
        props: (route: any) => ({
            vodUuid: route.params.vodUuid,
            share: route.query.share,
        })
    },
    {
        path: '/setupwizard',
        name: pi.SetupWizardPageId,
        component: SetupWizard,
    },
    {
        path: '/',
        component: isDesktop() ? PrimaryApp : WebApp,
        props: (route: any) => ({
            showNav: !route.query.nonav || parseInt(route.query.nonav) === 0
        }),
        children: [
            {
                path: '',
                name: pi.DashboardPageId,
                component: Dashboard,
                props: (route : any) => ({
                    needSetup: parseInt(route.query.needSetup),
                })
            },
            {
                path: 'postgame',
                name: pi.PostGameReportPageId,
                component: PostGameReport,
                props: (route : any) => ({
                    game: parseInt(route.query.game),
                    tm: new Date(parseInt(route.query.tm)),
                })
            },
            { 
                path: 'login',
                name: pi.LoginPageId,
                component: Login,
                props: (route : any) => ({
                    reg: route.query.reg,
                    redirect: route.query.redirect,
                    referral: route.query.ref,
                    needSetup: parseInt(route.query.needSetup),
                })
            },
            {
                path: 'register',
                name: pi.RegisterPageId,
                component: Register,
                props: (route : any) => ({
                    inviteUuid: route.query.inviteUuid,
                    squadId: parseInt(route.query.squadId),
                    sig: route.query.sig,
                    redirect: route.query.redirect,
                    referral: route.query.ref,
                })
            },
            {
                path: 'forgotpw/:changePasswordId',
                name: pi.ForgotPasswordPageId,
                component: ForgotPassword,
                props: (route : any) => ({
                    changePasswordId: route.params.changePasswordId
                })
            },
            { 
                path: 'verify/:verificationId',
                name: pi.VerifyEmailPageId,
                component: VerifyEmail,
                props: (route : any) => ({
                    verificationId: route.params.verificationId
                })
            },
            {
                path: 'profile/:userId/squads',
                name: pi.UserSquadsPageId,
                component: UserSquads,
                props: (route: any) => ({
                    userId: parseInt(route.params.userId),
                })
            },
            {
                path: 'squad/:squadId',
                name: pi.SingleSquadPageId,
                component: SingleSquadPage,
                props: (route: any) => ({
                    squadId: parseInt(route.params.squadId)
                })
            },
            {
                path: 'localstorage',
                name: pi.LocalStoragePageId,
                component: LocalStorageManager,
            },
            {
                path: 'watchlist',
                name: pi.WatchlistPageId,
                component: UserWatchlistPage,
            },
            {
                path: 'favorites',
                name: pi.FavoritesPageId,
                component: UserFavoritesPage,
            },
            { 
                path: 'logs/:userId',
                component: GameLog,
                props: (route: any) => ({
                    squadId: parseInt(route.query.squadId),
                    userId: parseInt(route.params.userId),
                }),
                children: [
                    {
                        path: 'all',
                        name: pi.LogPageId,
                        component: LogGameChooser,
                        props: (route : any) => ({
                            userId: parseInt(route.params.userId),
                        })
                    },
                    {
                        path: 'csgo',
                        name: pi.CsgoLogPageId,
                        component: CsgoGameLog,
                        props: (route : any) => ({
                            userId: parseInt(route.params.userId),
                        })
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
                        path: 'wow/:release/character/:guid?',
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
                                            release: wowGameReleaseFromString(route.params.release),
                                        }),
                                    },
                                    {
                                        path: 'challenges',
                                        name: pi.WowChallengeLogPageId,
                                        component: WowKeystoneGameLog,
                                        props: (route : any) => ({
                                            userId: parseInt(route.params.userId),
                                            guid: route.params.guid,
                                            release: wowGameReleaseFromString(route.params.release),
                                        }),
                                    },
                                    {
                                        path: 'arena',
                                        name: pi.WowArenaLogPageId,
                                        component: WowArenaGameLog,
                                        props: (route : any) => ({
                                            userId: parseInt(route.params.userId),
                                            guid: route.params.guid,
                                            release: wowGameReleaseFromString(route.params.release),
                                        }),
                                    }
                                ]
                            },
                        ],
                        component: WowLogContainer,
                        props: (route : any) => ({
                            userId: parseInt(route.params.userId),
                            guid: route.params.guid,
                            release: wowGameReleaseFromString(route.params.release),
                        }),
                    }
                ],
            },
            {
                path: 'match/csgo/:matchUuid',
                name: pi.CsgoMatchPageId,
                component: CsgoMatch,
                props: (route: any) => ({
                    matchUuid: route.params.matchUuid,
                    userId: parseInt(route.query.userId),
                    inTs: timeStringToSeconds(route.query.t),
                }),
            },
            {
                path: 'match/valorant/:matchUuid',
                name: pi.ValorantMatchPageId,
                component: ValorantMatch,
                props: (route : any) => ({
                    puuid: route.query.account,
                    matchUuid: route.params.matchUuid,
                    userId: parseInt(route.query.userId),
                    inTs: timeStringToSeconds(route.query.t),
                })
            },
            {
                path: 'match/tft/:matchUuid',
                name: pi.TftMatchPageId,
                component: TftMatch,
                props: (route : any) => ({
                    puuid: route.query.account,
                    matchUuid: route.params.matchUuid,
                    userId: parseInt(route.query.userId),
                    inTs: timeStringToSeconds(route.query.t),
                })
            },
            {
                path: 'match/lol/:matchUuid',
                name: pi.LolMatchPageId,
                component: LolMatch,
                props: (route : any) => ({
                    matchUuid: route.params.matchUuid,
                    userId: parseInt(route.query.userId),
                    inTs: timeStringToSeconds(route.query.t),
                })
            },
            {
                path: 'match/aimlab/:taskId',
                name: pi.AimlabMatchPageId,
                component: AimlabMatch,
                props: (route : any) => ({
                    taskId: route.params.taskId,
                    userId: parseInt(route.query.userId),
                    inTs: timeStringToSeconds(route.query.t),
                })
            },
            {
                path: 'match/hearthstone/:matchId',
                name: pi.HearthstoneMatchPageId,
                component: HearthstoneMatch,
                props: (route : any) => ({
                    matchId: route.params.matchId,
                    userId: parseInt(route.query.userId),
                    inTs: timeStringToSeconds(route.query.t),
                })
            },
            {
                path: 'match/wow/:matchUuid',
                name: pi.WowMatchPageId,
                component: WowMatch,
                props: (route : any) => ({
                    userId: parseInt(route.query.userId),
                    matchUuid: route.params.matchUuid,
                    inTs: timeStringToSeconds(route.query.t),
                }),
            },
            {
                path: 'performance',
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
                path: 'riot/oauth-callback',
                component: RsoOauthHandler,
                name: pi.RsoOauthPageId,
                props: (route : any) => ({
                    code: route.query.code,
                    state: route.query.state,
                }),
            },
            {
                path: 'twitch/oauth-callback',
                component: TwitchOauthHandler,
                name: pi.TwitchOauthPageId,
                props: (route : any) => ({
                    code: route.query.code,
                    state: route.query.state,
                }),
            },
            {
                path: 'invite/:inviteUuid/:action',
                component: SquadInviteResponsePage,
                name: pi.InviteResponsePageId,
                props: (route: any) => ({
                    inviteUuid: route.params.inviteUuid,
                    action: route.params.action,
                    isUser: (route.query.isUser === 'true'),
                    squadId: parseInt(route.query.squadId),
                    sig: route.query.sig,
                })
            },
            {
                path: 'share/:accessTokenId',
                component: ShareRedirect,
                name: pi.ShareRedirectPageId,
                props: (route: any) => ({
                    accessTokenId: route.params.accessTokenId,
                })
            },
            {
                path: 'settings',
                component: AppSettingsPage,
                name: pi.SettingsPageId,
                props: (route: any) => ({
                    inputTab: route.query.inputTab,
                })
            },
            {
                path: 'clip',
                name: pi.ClipLibraryPageId,
                component: ClipLibrary,
                props: (route: any) => ({
                    matchUuid: route.query.matchUuid,
                    refresh: parseInt(route.query.refresh) === 1
                })
            },
            {
                path: 'clip/:clipUuid',
                name: pi.ClipPageId,
                component: ClipView,
                props: (route: any) => ({
                    clipUuid: route.params.clipUuid,
                    inTs: timeStringToSeconds(route.query.t),
                })
            },
            {
                path: 'c',
                component: Community,
                children: [
                    {
                        path: '',
                        name: pi.CommunityHomePageId,
                        component: CommunityFeed,
                    },
                    {
                        path: 'discover',
                        name: pi.CommunityDiscoverPageId,
                        component: CommunityDiscover,
                        props: (route: any) => ({
                            search: route.query.search
                        })
                    },
                    {
                        path: ':communitySlug',
                        name: pi.CommunityPageId,
                        component: CommunityPage,
                        props: (route: any) => ({
                            slug: route.params.communitySlug
                        })
                    }
                ]
            },
            {
                path: 'p/:profileSlug',
                name: pi.UserProfileSlugPageId,
                component: UserProfile,
                props: (route : any) => ({
                    profileSlug: route.params.profileSlug,
                })
            },
            {
                path: 'pi/:userId',
                name: pi.UserProfileIdPageId,
                component: UserProfile,
                props: (route : any) => ({
                    userId: parseInt(route.params.userId),
                })
            },
            {
                path: 'link/:linkId',
                component: SquadLinkResponsePage,
                name: pi.LinkResponsePageId,
                props: (route: any) => ({
                    linkId: route.params.linkId
                })
            },
        ]
    }
]

const router = new VueRouter({
/// #if !DESKTOP
    mode :'history',
/// #endif
    routes: baseRoutes,
})

const store = new Vuex.Store(RootStoreOptions)

import * as Sentry from '@sentry/vue'
import { Integrations } from '@sentry/tracing'
import { CaptureConsole, Offline } from '@sentry/integrations'
import { version } from '@client/package.json'

function getSentryRelease(): string {
    let v = version
    if (!process.env.NODE_ENV || process.env.NODE_ENV === 'development') {
        v = 'dev'
    }

///#if !DESKTOP
    return `squadov-common-web-browser@${v}`
///#else
    return `squadov-common-web-desktop@${v}`
///#endif
}

function initializeSentry(u: SquadOVUser) {
    if (!!process.env.NODE_ENV && process.env.NODE_ENV !== 'development') {
        apiClient.getSentryDsn().then((resp: ApiData<string>) => {
            Sentry.init({
                Vue,
                dsn: resp.data,
                maxBreadcrumbs: 500,
                release: getSentryRelease(),
                integrations: [
                    new Integrations.BrowserTracing({
                        routingInstrumentation: Sentry.vueRouterInstrumentation(router),
                        tracingOrigins: ['localhost', 'app.squadov.gg', /^\//],
                    }),
                    new CaptureConsole({
                        levels: ['error']
                    }),
                    new Offline(),
                ],
                tracesSampleRate: 0.1,
                trackComponents: true,
            })

            Sentry.setUser({
                id: u.uuid,
                username: u.username,
                ip_address: '{{auto}}',
            })
        }).catch((err: any) => {
            console.error('Failed to get Sentry DSN: ', err)
        })
    }
}

import { initializeAnalyticsContainer, getAnalyticsContainer } from '@client/js/analytics/container'
import { TrackedUserStatsManager } from '@client/js/squadov/status'
const statusTracker = new TrackedUserStatsManager(store)

import { loadInitialSessionFromCookies, checkHasSessionCookie } from '@client/js/session'
import { WowGameRelease, wowGameReleaseFromString } from './staticData';
import { SquadOvGames } from './squadov/game';

router.afterEach((to: Route, from: Route) => {
    let container = getAnalyticsContainer()
    if (!!container) {
        container.pageView(to)
    }
})

router.beforeEach((to : Route, from : Route, next : any) => {
    console.log(`Navigate ${from.fullPath} (${from.name}) => ${to.fullPath} (${to.name})`)
    
    let mustBeInvalid = (to.name === pi.LoginPageId || to.name === pi.RegisterPageId)

    // Certain pages should be allowed to be public would be nice to make this somehow less hard-coded or something...
    let isPublic = to.name === pi.RsoOauthPageId
        || to.name === pi.TwitchOauthPageId
        || to.name === pi.InviteResponsePageId 
        || to.name === pi.ShareRedirectPageId
        || to.name === pi.VerifyEmailPageId
        || to.name === pi.PlayerPageId
        || to.name === pi.ForgotPasswordPageId
        || to.name === pi.UserProfileSlugPageId

    // the domain here doesn't matter as we don't use it.
    let nextUrl = new URL(to.fullPath, 'http://localhost')
    apiClient.setTempSessionId(nextUrl.searchParams.get('share'), nextUrl.searchParams.get('uid'))
    apiClient.setAccessToken(nextUrl.searchParams.get('at'))

    let isTmpSession = apiClient.hasTempSession || !!apiClient._accessToken
    let isAuth = !!store.state.currentUser

/// #if DESKTOP
    next()
/// #else
    let hasCookie = checkHasSessionCookie()
    if (hasCookie && !isAuth) {
        // If we have a cookie and the current user isn't set, then send out a request
        // to verify the session cookie.
        loadInitialSessionFromCookies(store, () => {
            initializeAnalyticsContainer(store)
            apiClient.getIpAddress().then((resp: ApiData<string>) => {
                getAnalyticsContainer()?.identify(resp.data)
            }).catch((err: any) => {
                console.error('Failed to get IP address: ', err)
            })

            store.dispatch('loadUserFeatureFlags')
            initializeSentry(store.state.currentUser!)
            
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
            if (!mustBeInvalid && !isTmpSession && !isPublic) {
                next({
                    name: pi.LoginPageId,
                    query: {
                        redirect: to.fullPath,
                    }
                })
            } else {
                next()
            }
        })
    } else {
        if (isTmpSession || isPublic) {
            next()
        } else if (mustBeInvalid && hasCookie) {
            next({
                name: pi.DashboardPageId
            })
        } else if (!mustBeInvalid && !hasCookie) {
            next({
                name: pi.LoginPageId,
                query: {
                    redirect: to.fullPath,
                }
            })
        } else {
            next()
        }
    }
/// #endif
})

/// #if DESKTOP
ipcRenderer.invoke('request-app-folder').then((appFolder: string) => {
    process.env.SQUADOV_USER_APP_FOLDER = appFolder
    store.dispatch('reloadLocalSettings')
    initializeAnalyticsContainer(store)
})

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
        initializeSentry(resp.data)
        store.commit('setUser' , resp.data)
        store.dispatch('loadUserFeatureFlags')

        apiClient.getIpAddress().then((resp: ApiData<string>) => {
            getAnalyticsContainer()?.identify(resp.data)
        }).catch((err: any) => {
            console.error('Failed to get IP address: ', err)
        })
    }).catch((err : any ) => {
        // Uhhhhhhhhhhhhhhhhhhhhhhh....? Need to logout here since
        // the stored session is garbage and so we have no way to recover
        // unless the user re-logs in. There's a chance that the API might have failed
        // so don't bother trying to do a proper API logout.
        console.error('Failed to obtain user: ', err)
        ipcRenderer.sendSync('logout')
    })
})

ipcRenderer.on('service-error', () => {
    store.commit('notifyServiceError')
})

ipcRenderer.on('main-update-downloaded', () => {
    store.commit('notifyUpdate')
})

ipcRenderer.on('change-running-games', (_, games) => {
    store.commit('setRunningGames', games)
    statusTracker.refreshCurrentUserStatus()
})

ipcRenderer.on('change-recording-games', (_, games) => {
    store.commit('setRecordingGames', games)
    statusTracker.refreshCurrentUserStatus()
})

ipcRenderer.on('post-game-notification', (_, data: { game: SquadOvGames, tm: Date }) => {
    data.tm = new Date(data.tm)

    // Redirect to the post-game report screen.
    // In the case where no games exist on the server then that component will be responsible
    // for displaying something reasonable to the user.
    router.push({
        name: pi.PostGameReportPageId,
        query: {
            game: `${data.game}`,
            tm: `${data.tm.getTime()}`,
        }
    })
})

ipcRenderer.on('redirect-to-route', (_, route: any) => {
    router.push(route)
})

setInterval(() => {
    store.dispatch('recomputeLocalDiskSpaceRecordUsage')
}, 60000)

/// #endif

new Vue({
    el: '#app',
    components: {
        VApp,
        VMain,
        App,
        TitleBar,
    },
    vuetify: new Vuetify({
        theme: {
            dark: true,
        },
    }),
    data: () => ({
        statusTracker,
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

export function getActiveUserId(): number {
    return apiClient.hasTempSession ? apiClient._tempUserId! : store.state.currentUser?.id!
}