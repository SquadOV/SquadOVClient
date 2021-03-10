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

const baseRoutes : any[] = [
    {
        path: '/metrics/active-users',
        component: MetricActiveUsers,
    },
    {
        path: '/metrics/churn',
        component: MetricChurn,
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