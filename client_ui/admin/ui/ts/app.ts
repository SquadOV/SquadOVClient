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
import AppNav from '@client/vue/AppNav.vue'

const MetricActiveUsers = () => import('@client/vue/metrics/MetricActiveUsers.vue')
const MetricChurn = () => import('@client/vue/metrics/MetricChurn.vue')
const MetricRegistrations = () => import('@client/vue/metrics/MetricRegistrations.vue')
const MetricSquads = () => import('@client/vue/metrics/MetricSquads.vue')
const MetricSquadInvites = () => import('@client/vue/metrics/MetricSquadInvites.vue')
const MetricGames = () => import('@client/vue/metrics/MetricGames.vue')
const MetricVods = () => import('@client/vue/metrics/MetricVods.vue')
const MetricLostUsers = () => import('@client/vue/metrics/MetricLostUsers.vue')
const MetricAverageAge = () => import('@client/vue/metrics/MetricAverageAge.vue')

const ReferralUsers = () => import('@client/vue/referral/ReferralUsers.vue')
const ReferralCampaigns = () => import('@client/vue/referral/ReferralCampaigns.vue')
const KpiDashboard = () => import('@client/vue/KpiDashboard.vue')
const CohortInstalledUsers = () => import('@client/vue/cohorts/CohortInstalledUsers.vue')
const CohortActiveUsers = () => import('@client/vue/cohorts/CohortActiveUsers.vue')
const CohortRecordingUsers = () => import('@client/vue/cohorts/CohortRecordingUsers.vue')

const baseRoutes : any[] = [
    {
        path: '/',
        component: KpiDashboard,
    },
    {
        path: '/metrics/active-users',
        component: MetricActiveUsers,
    },
    {
        path: '/metrics/average-age',
        component: MetricAverageAge,
    },
    {
        path: '/metrics/lost-users',
        component: MetricLostUsers,
    },
    {
        path: '/metrics/churn',
        component: MetricChurn,
    },
    {
        path: '/metrics/registrations',
        component: MetricRegistrations,
    },
    {
        path: '/metrics/squads',
        component: MetricSquads,
    },
    {
        path: '/metrics/squadinvs',
        component: MetricSquadInvites,
    },
    {
        path: '/metrics/games',
        component: MetricGames,
    },
    {
        path: '/metrics/vods',
        component: MetricVods,
    },
    {
        path: '/referrals/users',
        component: ReferralUsers,
    },
    {
        path: '/referrals/campaigns',
        component: ReferralCampaigns,
    },
    {
        path: '/cohorts/installed-users',
        component: CohortInstalledUsers,
    },
    {
        path: '/cohorts/active-users',
        component: CohortActiveUsers,
    },
    {
        path: '/cohorts/recording-users',
        component: CohortRecordingUsers,
    }
]

const router = new VueRouter({
    mode :'history',
    routes: baseRoutes,
})

new Vue({
    el: '#app',
    components: {
        VApp,
        VMain,
        App,
        AppNav
    },
    vuetify: new Vuetify({
        theme: {
            dark: true,
        },
    }),
    methods: {
    },
    router,
}).$mount('#app')