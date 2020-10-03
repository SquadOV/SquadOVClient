import 'regenerator-runtime/runtime.js';
import '@mdi/font/css/materialdesignicons.css' 

import Vue from 'vue'
import Vuetify, {
    VApp,
    VMain,
} from 'vuetify/lib'

import PasswordChooser from '@client/vue/PasswordChooser.vue'

Vue.use(Vuetify)

new Vue({
    el: '#app',
    components: {
        VApp,
        VMain,
        PasswordChooser
    },
    vuetify: new Vuetify({
        theme: {
            dark: true,
        },
    }),
}).$mount('#app')