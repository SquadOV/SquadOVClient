import 'regenerator-runtime/runtime.js';
import '@mdi/font/css/materialdesignicons.css' 

import Vue from 'vue'
import Vuetify, {
    VApp,
    VMain,
    VProgressCircular
} from 'vuetify/lib'
import StatusDisplay from '@client/vue/utility/squadov/StatusDisplay.vue'

Vue.use(Vuetify)

new Vue({
    el: '#app',
    components: {
        VApp,
        VMain,
        VProgressCircular,
        StatusDisplay
    },
    vuetify: new Vuetify({
        theme: {
            dark: true,
        },
    }),
}).$mount('#app')