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
                    :input-value="syncToNtp"
                    @change="changeSyncToNtp"
                    dense
                    hide-details
                    label="Sync time to NTP servers"
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
            </v-tab-item>

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

@Component({
    components: {
        RecordingSettingsItem,
        AccountSecuritySettingsItem,
        PlaybackSettingsItem,
        SharingSettingsItem,
    }
})
export default class AppSettingsPage extends Vue {
    @Prop()
    inputTab!: number | undefined

    tab: number = 0

    @Watch('inputTab')
    syncFromInput() {
        if (this.inputTab !== undefined) {
            this.tab = this.inputTab
        }
    }

    mounted() {
        this.syncFromInput()
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

    get syncToNtp(): boolean {
        return this.$store.state.settings.enableNtp
    }

    changeSyncToNtp(v: boolean) {
        this.$store.commit('setEnableNtp', v)
    }

    get anonymousAnalytics(): boolean {
        return this.$store.state.settings.anonymousAnalytics
    }

    changeAnonymousAnalytics(v: boolean) {
        this.$store.commit('setAnonymousAnalytics', v)
    }
}

</script>