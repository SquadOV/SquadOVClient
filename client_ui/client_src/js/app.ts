import 'regenerator-runtime/runtime.js';
import '@mdi/font/css/materialdesignicons.css' 

import Vue from 'vue'
import Vuetify, {
    VApp,
    VMain,
} from 'vuetify/lib'
import VueRouter from 'vue-router'

Vue.use(Vuetify)
Vue.use(VueRouter)

import App from '@client/vue/App.vue'
import Dashboard from '@client/vue/Dashboard.vue'

import GameLog from '@client/vue/GameLog.vue'
import LogGameChooser from '@client/vue/log/LogGameChooser.vue'
import ValorantLogContainer from '@client/vue/log/ValorantLogContainer.vue'
import ValorantGameLog from '@client/vue/log/ValorantGameLog.vue'
import ValorantMatch from '@client/vue/log/ValorantMatch.vue'

import AimlabGameLog from '@client/vue/log/AimlabGameLog.vue'
import AimlabMatch from '@client/vue/log/AimlabMatch.vue'

import Performance from '@client/vue/Performance.vue'
import PerformanceComponentChooser from '@client/vue/performance/PerformanceComponentChooser.vue'
import VizStats from '@client/vue/performance/VizStats.vue'

import * as pi from '@client/js/pages'
import { ipcRenderer } from 'electron'
import { apiClient, ApiData } from '@client/js/api'
import { getSquadOVUser, SquadOVUser } from '@client/js/squadov/user'

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

// As soon as the app starts we need to query the main process for
// the session ID properly initialize the API client
// so that it's authenticated with the web server. After that, send out
// an immediate request to obtain the current user profile and store it in the
// Vuex storage to make it available to the frontend.
let session : {
    sessionId: string,
    userId: string
} = ipcRenderer.sendSync('request-session')
apiClient.setSessionId(session.sessionId)
getSquadOVUser(parseInt(session.userId)).then((resp : ApiData<SquadOVUser>) => {

}).catch((err : any ) => {
    // Uhhhhhhhhhhhhhhhhhhhhhhh....?
    console.log('Failed to obtain user: ', err)
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
}).$mount('#app')
console.log('Loading Vue Application')