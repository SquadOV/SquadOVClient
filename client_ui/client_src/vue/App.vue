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

            <v-banner
                v-if="$root.globals.hasUpdate"
                single-line
                sticky
                color="primary"
            >
                An update for SquadOV is available. Please restart SquadOV to install it.
            </v-banner>
            
            <div class="flex-grow-1">
                <router-view></router-view>
            </div>

            <footer class="flex-grow-0">
                <div class="text-center ma-0 text-overline footer-text">
                    SquadOV v{{ version }} - Created with <i class="fas fa-heart"></i> by gamers for gamers.
                </div>

                <div class="d-flex justify-center ma-0 text-overline">
                    <v-dialog v-model="showHideLicenses" persistent max-width="80%">
                        <template v-slot:activator="{on , attrs}">
                            <v-btn
                                color="primary"
                                x-small
                                text
                                v-on="on"
                                v-bind="attrs"
                            >
                                Licenses
                            </v-btn>
                        </template>
                        
                        <v-card>
                            <v-card-title>
                                Licenses
                            </v-card-title>
                            
                            <license-data></license-data>

                            <v-card-actions>
                                <v-spacer></v-spacer>
                                <v-btn
                                    color="error"
                                    @click="showHideLicenses = false"
                                >
                                    Close
                                </v-btn>
                            </v-card-actions>
                        </v-card>
                    </v-dialog>
                </div>
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
import LicenseData from '@client/vue/LicenseData.vue'
import { version } from '@client/../package.json'

@Component({
    components: {
        AppNav,
        LicenseData
    }
})
export default class App extends Vue {
    version: string = version
    showHideLicenses: boolean = false

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

.footer-text {
    line-height: 1rem !important;
}

</style>