import 'regenerator-runtime/runtime.js';
import '@mdi/font/css/materialdesignicons.css' 

import Vue from 'vue'
import Vuetify, {
    VApp,
    VMain,
} from 'vuetify/lib'

import PasswordMigrater from '@client/vue/PasswordMigrater.vue'

Vue.use(Vuetify)

new Vue({
    el: '#app',
    components: {
        VApp,
        VMain,
        PasswordMigrater
    },
    vuetify: new Vuetify({
        theme: {
            dark: true,
        },
    }),
}).$mount('#app')