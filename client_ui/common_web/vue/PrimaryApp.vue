<template>
    <div id="appInner">
        <template v-if="userLoaded">    
            <app-nav class="flex-grow-0" v-if="showNav"></app-nav>
            <app-alerts width="100%"></app-alerts>
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
                <div
                    class="d-flex align-center"
                >
                    An update for SquadOV is available. Please restart SquadOV to install it.
                    <v-spacer></v-spacer>
                    <v-btn
                        solo
                        @click="restart"
                    >
                        Restart Now
                    </v-btn>
                </div>
            </v-banner>
            
            <div class="d-flex flex-column flex-grow-1">
                <router-view></router-view>
            </div>

            <div id="footer" class="flex-grow-0 py-1" v-if="showNav">
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

                <v-dialog
                    v-model="showHideBug"
                    persistent
                    max-width="60%"
                >
                    <template v-slot:activator="{on, attrs}">
                        <v-btn
                            v-on="on"
                            v-bind="attrs"
                            small
                            id="bugreport"
                            color="error"
                        >
                            <v-icon>
                                mdi-bug
                            </v-icon>
                            Report a Bug
                        </v-btn>
                    </template>
                    <bug-reporter
                        @cancel="showHideBug = false"
                    ></bug-reporter>
                </v-dialog>
    
                <recording-status-window
                    id="recording"
                >
                </recording-status-window>
            </div>
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

import Component, {mixins} from 'vue-class-component'
import AppNav from '@client/vue/AppNav.vue'
import LicenseData from '@client/vue/LicenseData.vue'
import BugReporter from '@client/vue/BugReporter.vue'
import RecordingStatusWindow from '@client/vue/utility/RecordingStatusWindow.vue'
import rawData from '@client/package.json'
import TopLevelComponent from '@client/vue/TopLevelComponent'
import AppAlerts from '@client/vue/utility/squadov/alerts/AppAlerts.vue'

/// #if DESKTOP
import { ipcRenderer } from 'electron'
/// #endif

@Component({
    components: {
        AppNav,
        LicenseData,
        BugReporter,
        RecordingStatusWindow,
        AppAlerts,
    }
})
export default class PrimaryApp extends mixins(TopLevelComponent) {
    version: string = rawData.version
    showHideLicenses: boolean = false
    showHideBug: boolean = false
    
    get userLoaded(): boolean {
        return !!this.$store.state.currentUser
    }
    
    restart() {
/// #if DESKTOP
        ipcRenderer.send('request-restart')
/// #endif
    }
}

</script>

<style scoped>

#bugreport {
    position: fixed;
    bottom: 0px;
    left: calc(100% - 250px);
    width: 180px;
    border-top-left-radius: 5px;
    border-top-right-radius: 5px;
    border-bottom-left-radius: 0px;
    border-bottom-right-radius: 0px;
}

#recording {
    position: fixed;
    bottom: 0px;
    left: 10px;
    width: 360px;
    border-top-left-radius: 5px;
    border-top-right-radius: 5px;
    border-bottom-left-radius: 0px;
    border-bottom-right-radius: 0px;
}

</style>