<template>
    <div id="appInner">
        <app-nav class="flex-grow-0" v-if="hasValidSession"></app-nav>            
        <div class="d-flex flex-column flex-grow-1">
            <router-view v-if="hasValidSession"></router-view>
            <template v-else>
                <div class="d-flex full-height full-width justify-center align-center flex-column">
                    <v-progress-circular indeterminate size="64">
                    </v-progress-circular>

                    <div class="text-h6 my-4">
                        An error occurred while connecting to SquadOV...Trying to reconnect.
                    </div>

                    <status-display style="width: 500px;"></status-display>
                </div>
            </template>

            <div id="footer" class="flex-grow-0 py-1">
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
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import AppNav from '@client/vue/AppNav.vue'
import { version } from '@client/package.json'

@Component({
    components: {
        AppNav,
    }
})
export default class WebApp extends Vue {
    version: string = version
    showHideLicenses: boolean = false

    get hasValidSession(): boolean {
        return this.$store.state.hasValidSession
    }
}

</script>