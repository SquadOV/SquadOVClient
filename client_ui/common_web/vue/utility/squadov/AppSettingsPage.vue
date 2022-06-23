<template>
    <v-container class="full-parent-height" fluid>
        <v-tabs class="full-parent-height" vertical v-model="tab">
            <v-tab>
                <v-icon left>
                    mdi-cog
                </v-icon>
                General
            </v-tab>

            <v-tab-item>
                <v-checkbox
                    :input-value="runOnStartup"
                    @change="changeRunOnStartup"
                    dense
                    hide-details
                    label="Run SquadOV on startup"
                >
                </v-checkbox>

                <v-checkbox
                    :input-value="minimizeToTray"
                    @change="changeMinimizeToTray"
                    dense
                    hide-details
                    label="Minimize SquadOV to system tray"
                >
                </v-checkbox>

                <v-checkbox
                    :input-value="minimizeOnClose"
                    @change="changeMinimizeOnClose"
                    dense
                    hide-details
                    label="Minimize SquadOV on close"
                >
                </v-checkbox>

                <v-checkbox
                    :input-value="anonymousAnalytics"
                    @change="changeAnonymousAnalytics"
                    dense
                    hide-details
                    label="Send Anonymous Usage Data to SquadOV"
                >
                </v-checkbox>

                <v-checkbox
                    :input-value="$store.state.settings.useHwAccel"
                    @change="$store.commit('changeUseHwAccel', arguments[0])"
                    dense
                    hide-details
                    label="Use Hardware Acceleration (UI Only)"
                >
                </v-checkbox>

                <v-checkbox
                    :input-value="$store.state.settings.hideRecommendedSquads"
                    @change="$store.commit('changeHideRecommendedSquads', arguments[0])"
                    dense
                    hide-details
                    label="Hide Recommended Squads"
                >
                </v-checkbox>

                <v-checkbox
                    :input-value="$store.state.settings.useStaticThumbnails"
                    @change="$store.commit('changeUseStaticThumbnails', arguments[0])"
                    dense
                    hide-details
                    label="Use Static Thumbnails"
                >
                </v-checkbox>
            </v-tab-item>

             <v-tab>
                <v-icon left>
                    mdi-gamepad
                </v-icon>
                Games
            </v-tab>

            <v-tab-item>
                <per-game-settings-item></per-game-settings-item>
            </v-tab-item>

            <v-tab>
                <v-icon left>
                    mdi-account
                </v-icon>
                Accounts
            </v-tab>

            <v-tab-item>
                <linked-accounts-settings-item></linked-accounts-settings-item>
            </v-tab-item>

            <template v-if="useRecordingSettings">
                <v-tab>
                    <v-icon left>
                        mdi-palette
                    </v-icon>
                    Overlays
                </v-tab>

                <v-tab-item>
                    <overlay-settings-item></overlay-settings-item>
                </v-tab-item>
            </template>

            <v-tab>
                <v-icon left>
                    mdi-play-circle
                </v-icon>
                Playback
            </v-tab>

            <v-tab-item>
                <playback-settings-item></playback-settings-item>
            </v-tab-item>

            <template v-if="useRecordingSettings">
                <v-tab>
                    <v-icon left>
                        mdi-video-image
                    </v-icon>
                    Recording
                </v-tab>

                <v-tab-item>
                    <recording-settings-item></recording-settings-item>
                </v-tab-item>
            </template>

            <v-tab>
                <v-icon left>
                    mdi-security
                </v-icon>
                Security
            </v-tab>

            <v-tab-item>
                <account-security-settings-item></account-security-settings-item>
            </v-tab-item>

            <v-tab>
                <v-icon left>
                    mdi-share-variant
                </v-icon>
                Sharing
            </v-tab>

            <v-tab-item>
                <div class="my-2">
                    Manage your auto-sharing settings for your matches and clips on this page.
                    Only your matches with recorded VODs and your created clips are auto-shared, not those that are shared with you.
                </div>

                <sharing-settings-item></sharing-settings-item>
            </v-tab-item>

            <v-tab>
                <v-icon left>
                    mdi-monitor
                </v-icon>
                In-Game
            </v-tab>

            <v-tab-item>
                <in-game-settings-item></in-game-settings-item>
            </v-tab-item>

            <v-tab @click="goToSubManagement" ripple>
                <v-icon left>
                    mdi-currency-usd
                </v-icon>
                Subscription
            </v-tab>

            <v-tab-item class="full-tab-height">
                <div class="d-flex justify-center flex-column align-center full-width full-parent-height">
                    <v-progress-circular indeterminate size="64" v-if="manageInProgress"></v-progress-circular>
                    <template v-else-if="manageFail">
                        <div class="text-h4 font-weight-bold">Oops, either you are not subscribed or you have never subscribed in the past.</div>
                        <v-btn
                            exact
                            :to="pricingTo"
                            color="success"
                            class="mt-4"
                        >
                            Check out SquadOV Pro
                        </v-btn>
                    </template>
                </div>
            </v-tab-item>
        </v-tabs>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import RecordingSettingsItem from '@client/vue/utility/squadov/settings/RecordingSettingsItem.vue'
import AccountSecuritySettingsItem from '@client/vue/utility/squadov/settings/AccountSecuritySettingsItem.vue'
import PlaybackSettingsItem from '@client/vue/utility/squadov/settings/PlaybackSettingsItem.vue'
import SharingSettingsItem from '@client/vue/utility/squadov/settings/SharingSettingsItem.vue'
import LinkedAccountsSettingsItem from '@client/vue/utility/squadov/settings/LinkedAccountsSettingsItem.vue'
import PerGameSettingsItem from '@client/vue/utility/squadov/settings/PerGameSettingsItem.vue'
import OverlaySettingsItem from '@client/vue/utility/squadov/settings/OverlaySettingsItem.vue'
import InGameSettingsItem from '@client/vue/utility/squadov/settings/InGameSettingsItem.vue'
import { apiClient, ApiData } from '@client/js/api'
import { PricingPageId } from '@client/js/pages'

@Component({
    components: {
        RecordingSettingsItem,
        AccountSecuritySettingsItem,
        PlaybackSettingsItem,
        SharingSettingsItem,
        LinkedAccountsSettingsItem,
        PerGameSettingsItem,
        OverlaySettingsItem,
        InGameSettingsItem,
    }
})
export default class AppSettingsPage extends Vue {
    @Prop()
    inputTab!: number | undefined

    tab: number = 0
    manageFail: boolean = false
    manageInProgress: boolean = false

    get pricingTo(): any {
        return {
            name: PricingPageId
        }
    }

    @Watch('inputTab')
    syncFromInput() {
        if (this.inputTab !== undefined) {
            this.tab = this.inputTab
        }
    }

    mounted() {
        this.syncFromInput()
    }

    goToSubManagement() {
        this.manageInProgress = true
        apiClient.manageSubscription().then((resp: ApiData<string>) => {
            window.location.href = resp.data
        }).catch((err: any) => {
            console.log('Failed to manage subscription: ', err)
            this.manageFail = true
        }).finally(() => {
            this.manageInProgress = false
        })
    }

    get useRecordingSettings(): boolean{
///#if DESKTOP
        return true
///#else
        return false
///#endif
    }

    get runOnStartup(): boolean {
        return this.$store.state.settings.runOnStartup
    }

    changeRunOnStartup(v: boolean) {
        this.$store.commit('setRunOnStartup', v)
    }

    get minimizeToTray() : boolean {
        return this.$store.state.settings.minimizeToTray
    }

    changeMinimizeToTray(v: boolean) {
        this.$store.commit('setMinimizeToSystemTray', v)
    }

    get minimizeOnClose() : boolean {
        return this.$store.state.settings.minimizeOnClose
    }

    changeMinimizeOnClose(v: boolean) {
        this.$store.commit('setMinimizeOnClose', v)
    }

    get anonymousAnalytics(): boolean {
        return this.$store.state.settings.anonymousAnalytics
    }

    changeAnonymousAnalytics(v: boolean) {
        this.$store.commit('setAnonymousAnalytics', v)
    }
}

</script>

<style scoped>

>>>.v-window__container {
    height: 100%;
}

>>>.full-tab-height {
    height: 100%;
}

</style>