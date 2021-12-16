import { RootStoreOptions } from '@client/js/vuex/store'
import Vuex from 'vuex'
import 'regenerator-runtime/runtime.js';
import '@mdi/font/css/materialdesignicons.css' 
import SetupProgress from '@client/vue/SetupProgress.vue'

import Vue from 'vue'
import Vuetify, {
    VApp,
    VMain,
} from 'vuetify/lib'
/// #if DESKTOP
import { ipcRenderer } from 'electron'
/// #endif
Vue.use(Vuetify)
Vue.use(Vuex)

ipcRenderer.invoke('request-app-folder').then((appFolder: string) => {
    process.env.SQUADOV_USER_APP_FOLDER = appFolder
    store.dispatch('reloadLocalSettings')
})

const store = new Vuex.Store(RootStoreOptions)
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
    store: store,
}).$mount('#app')