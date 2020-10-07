import 'regenerator-runtime/runtime.js';
import '@mdi/font/css/materialdesignicons.css' 

import Vue from 'vue'
import Vuetify, {
    VApp,
    VMain,
} from 'vuetify/lib'
import VueRouter from 'vue-router'

import Login from '@client/vue/auth/Login.vue'
import Register from '@client/vue/auth/Register.vue'

Vue.use(Vuetify)
Vue.use(VueRouter)
const baseRoutes : any[] = [
    { path: '/', name: 'login', component: Login },
    { path: '/register', name: 'register', component: Register },
]

const router = new VueRouter({
    routes: baseRoutes,
})

new Vue({
    el: '#app',
    components: {
        VApp,
        VMain,
        Login,
        Register
    },
    vuetify: new Vuetify({
        theme: {
            dark: true,
        },
    }),
    router,
}).$mount('#app')