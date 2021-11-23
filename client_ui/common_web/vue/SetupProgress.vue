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
import Funnies from 'funnies'

@Component
export default class SetupProgress extends Vue {
    statusMessage = 'Loading...'
    showLogout: boolean = false
    funnies: any = new Funnies()

    refreshStatusMessage() {
        this.statusMessage = this.funnies.message()
    }

    async generateSettingsFile() {
        console.log('Generating settings file...')
        this.refreshStatusMessage()
        let settings = await loadLocalSettings()
        return settings
    }

    async audioDeviceSanityCheck() {
        console.log('Audio device sanity check...')
        this.refreshStatusMessage()

        let p = new Promise((resolve, reject) => {
            ipcRenderer.on('finish-audio-devices-sanity-check', () => {
                resolve(0)
            })
        })

        ipcRenderer.send('audio-devices-sanity-check')
        return p
    }

    logout() {
        ipcRenderer.send('logout')
    }

    async doSetupSequence() {
        // 1. The settings.json needs to be generated properly for any of the next steps to happen.
        await this.generateSettingsFile()

        // 2. Audio device sanity check. We need this primarily to transition the user from an ID-less
        //    settings file (for audio devices) and to backfill in the proper ID based on the given audio device name.
        //    The flip side of doing this will allow us to do an audio device sanity check every time we start up.
        await this.audioDeviceSanityCheck() 

        this.statusMessage = 'Connecting to SquadOV...'

        // Temporary measure against the white screen
        setTimeout(() => {
            this.showLogout = true
        }, 5000)

        ipcRenderer.send('finish-setup')
    }

    mounted() {
        this.doSetupSequence()
    }
}

</script>

<style scoped>

.setup-div {
    width: 300px;
    height: 300px;
}

</style>