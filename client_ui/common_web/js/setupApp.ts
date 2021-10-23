import 'regenerator-runtime/runtime.js';
import '@mdi/font/css/materialdesignicons.css' 
import SetupProgress from '@client/vue/SetupProgress.vue'

import Vue from 'vue'
import Vuetify, {
    VApp,
    VMain,
} from 'vuetify/lib'

Vue.use(Vuetify)
new Vue({
    el: '#app',
    components: {
        VApp,
        VMain,
        SetupProgress,
    },
    vuetify: new Vuetify({
        theme: {
            dark: true,
        },
    }),
}).$mount('#app')