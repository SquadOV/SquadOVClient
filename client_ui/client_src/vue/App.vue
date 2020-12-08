<template>
    <div id="appInner">
        <template v-if="userLoaded">    
            <app-nav class="flex-grow-0"></app-nav>
            <v-banner
                v-if="$root.globals.serviceError"
                single-line
                sticky
                color="error"
            >
                The local service encountered an unexpected error, please restart SquadOV and try again.
            </v-banner>
            
            <div class="flex-grow-1">
                <router-view></router-view>
            </div>

            <footer class="flex-grow-0">
                <p class="text-center ma-0 text-overline">
                    SquadOV v{{ version }} - Created with <i class="fas fa-heart"></i> by GRCHive, Inc.
                </p>
            </footer>
        </template>
        
        <v-container v-else>
            <v-row justify="center" align="center">
                <v-progress-circular
                    size="64"
                    indeterminate
                >
                </v-progress-circular>
            </v-row>
        </v-container>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import AppNav from '@client/vue/AppNav.vue'
import { version } from '@client/../package.json'

@Component({
    components: {
        AppNav
    }
})
export default class App extends Vue {
    version: string = version

    get userLoaded(): boolean {
        return !!this.$store.state.currentUser
    }
}

</script>

<style scoped>

#appInner {
    display: flex;
    flex-direction: column;
    height: 100%;
}

</style>