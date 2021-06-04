import 'regenerator-runtime/runtime.js';
import '@mdi/font/css/materialdesignicons.css' 

import Vue from 'vue'
import Vuetify, {
    VApp,
    VMain,
    VProgressCircular
} from 'vuetify/lib'
import StatusDisplay from '@client/vue/utility/squadov/StatusDisplay.vue'
import LogoutButton from '@client/vue/utility/auth/LogoutButton.vue'
import TitleBar from '@client/vue/TitleBar.vue'

Vue.use(Vuetify)

new Vue({
    el: '#app',
    components: {
        VApp,
        VMain,
        VProgressCircular,
        StatusDisplay,
        LogoutButton,
        TitleBar
    },
    vuetify: new Vuetify({
        theme: {
            dark: true,
        },
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
}).$mount('#app')