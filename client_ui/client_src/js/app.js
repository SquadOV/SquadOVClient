import 'regenerator-runtime/runtime.js';

import Vue from 'vue'
import Vuetify, {
    VApp,
    VMain,
} from 'vuetify/lib'
Vue.use(Vuetify)

import HomePage from '@client/vue/HomePage.vue'

new Vue({
    el: '#app',
    components: {
        VApp,
        VMain,
        HomePage,
    },
    vuetify: new Vuetify({}),
}).$mount('#app')