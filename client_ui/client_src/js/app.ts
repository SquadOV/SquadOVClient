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

import App from '@client/vue/App.vue'
import Dashboard from '@client/vue/Dashboard.vue'

import GameLog from '@client/vue/GameLog.vue'
import LogGameChooser from '@client/vue/log/LogGameChooser.vue'
import ValorantLogContainer from '@client/vue/log/ValorantLogContainer.vue'
import ValorantGameLog from '@client/vue/log/ValorantGameLog.vue'
import ValorantMatch from '@client/vue/log/ValorantMatch.vue'

import AimlabGameLog from '@client/vue/log/AimlabGameLog.vue'
import AimlabMatch from '@client/vue/log/AimlabMatch.vue'

import HearthstoneGameLog from '@client/vue/log/HearthstoneGameLog.vue'
import HearthstoneMatch from '@client/vue/log/HearthstoneMatch.vue'

import Performance from '@client/vue/Performance.vue'
import PerformanceComponentChooser from '@client/vue/performance/PerformanceComponentChooser.vue'
import VizStats from '@client/vue/performance/VizStats.vue'

import * as pi from '@client/js/pages'
import { ipcRenderer } from 'electron'
import { apiClient, ApiData } from '@client/js/api'
import { getSquadOVUser, SquadOVUser } from '@client/js/squadov/user'
import { RootStoreOptions } from '@client/js/vuex/store'

const baseRoutes : any[] = [
    { path: '/', name: pi.DashboardPageId, component: Dashboard },
    { 
        path: '/logs',
        component: GameLog,
        children: [
            {
                path: '/',
                name: pi.LogPageId,
                component: LogGameChooser
            },
            {
                path: '/valorant/account/:account?',
                children: [
                    {
                        path: '/',
                        name: pi.ValorantLogPageId,
                        component: ValorantGameLog,
                    },
                ],
                component: ValorantLogContainer,
            },
            {
                path: '/valorant/match/:matchId',
                name: pi.ValorantMatchPageId,
                component: ValorantMatch,
                props: (route : any) => ({
                    puuid: route.query.account,
                    matchId: route.params.matchId
                })
            },
            {
                path: '/aimlab',
                name: pi.AimlabLogPageId,
                component: AimlabGameLog,
            },
            {
                path: '/aimlab/task/:taskId',
                name: pi.AimlabMatchPageId,
                component: AimlabMatch,
                props: (route : any) => ({
                    taskId: route.params.taskId
                })
            },
            {
                path: '/hearthstone',
                name: pi.HearthstoneLogPageId,
                component: HearthstoneGameLog
            },
            {
                path: '/hearthstone/match/:matchId',
                name: pi.HearthstoneMatchPageId,
                component: HearthstoneMatch,
                props: (route : any) => ({
                    matchId: route.params.matchId
                })
            }
        ],
    },
    { 
        path: '/performance',
        component: Performance,
        children: [
            {
                path: '/',
                name: pi.PerformancePageId,
                component: PerformanceComponentChooser,
            },
            {
                path: '/visualization',
                name: pi.VizStatsPageId,
                component: VizStats,
            },
        ]
    },
]

const router = new VueRouter({
    routes: baseRoutes,
})

router.beforeEach((to : any, from : any, next : any) => {
    console.log(`Navigate ${from.path} => ${to.path}`)
    next()
})

const store = new Vuex.Store(RootStoreOptions)

// As soon as the app starts we need to query the main process for
// the session ID properly initialize the API client
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

new Vue({
    el: '#app',
    components: {
        VApp,
        VMain,
        App,
    },
    vuetify: new Vuetify({
        theme: {
            dark: true,
        },
    }),
    router: router,
    store: store,
}).$mount('#app')
console.log('Loading Vue Application')