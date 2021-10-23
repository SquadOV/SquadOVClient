<template>
    <div class="setup-div d-flex justify-center align-center flex-column pa-4">
        <v-progress-circular class="mb-4" indeterminate size="64">
        </v-progress-circular>

        <div>
            {{ statusMessage }}
        </div>

        <template v-if="showLogout">
            <div class="text-center mt-4">
                Is this taking a little longer than usual? Try logging out and logging back in.
            </div>

            <v-btn
                class="mt-2"
                color="error"
                @click="logout"
            >
                Logout
            </v-btn>
        </template>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { ipcRenderer } from 'electron'
import { loadLocalSettings } from '@client/js/system/settings'

@Component
export default class SetupProgress extends Vue {
    statusMessage = 'Loading...'
    showLogout: boolean = false

    async generateSettingsFile() {
        this.statusMessage = 'Generating settings file...'
        let settings = await loadLocalSettings()
        return settings
    }

    logout() {
        ipcRenderer.send('logout')
    }

    async mounted() {
        // 1. The settings.json needs to be generated properly for any of the next steps to happen.
        await this.generateSettingsFile()

        // 2. TODO...

        this.statusMessage = 'Connecting to SquadOV...'

        // Temporary measure against the white screen
        setTimeout(() => {
            this.showLogout = true
        }, 5000)

        ipcRenderer.send('finish-setup')
    }
}

</script>

<style scoped>

.setup-div {
    width: 300px;
    height: 300px;
}

</style>