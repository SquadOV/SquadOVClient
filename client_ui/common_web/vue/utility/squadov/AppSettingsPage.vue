<template>
    <v-container class="full-parent-height" fluid>
        <v-tabs class="full-parent-height" vertical>
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
                    mdi-cog
                </v-icon>
                System
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
            </v-tab-item>
        </v-tabs>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import RecordingSettingsItem from '@client/vue/utility/squadov/settings/RecordingSettingsItem.vue'
import AccountSecuritySettingsItem from '@client/vue/utility/squadov/settings/AccountSecuritySettingsItem.vue'
import PlaybackSettingsItem from '@client/vue/utility/squadov/settings/PlaybackSettingsItem.vue'

@Component({
    components: {
        RecordingSettingsItem,
        AccountSecuritySettingsItem,
        PlaybackSettingsItem,
    }
})
export default class AppSettingsPage extends Vue {
    
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
}

</script>