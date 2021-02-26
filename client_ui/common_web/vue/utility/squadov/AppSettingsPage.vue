<template>
    <v-container class="full-parent-height" fluid>
        <v-tabs class="full-parent-height" vertical>
            <v-tab>
                <v-icon left>
                    mdi-video-image
                </v-icon>
                Recording
            </v-tab>

            <v-tab-item>
                <recording-settings-item></recording-settings-item>
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
            </v-tab-item>
        </v-tabs>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import RecordingSettingsItem from '@client/vue/utility/squadov/settings/RecordingSettingsItem.vue'

@Component({
    components: {
        RecordingSettingsItem
    }
})
export default class AppSettingsPage extends Vue {

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
}

</script>