<template>
    <div class="setup-div d-flex justify-center align-center flex-column pa-4">
        <v-progress-circular class="mb-4" indeterminate size="64">
        </v-progress-circular>

        <div class="text-center mt-2">
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

        <template v-if="showSpeedCheckPrompt">
            <v-btn
                class="mt-4"
                color="success"
                @click="finishSpeedCheck"
            >
                Save & Continue
            </v-btn>
        </template>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { ipcRenderer } from 'electron'
import { loadLocalSettings, saveLocalSettings } from '@client/js/system/settings'
import Funnies from 'funnies'

@Component
export default class SetupProgress extends Vue {
    statusMessage = 'Loading...'
    showLogout: boolean = false
    funnies: any = new Funnies()
    showSpeedCheckPrompt: boolean = false

    get ranSpeedCheck(): boolean {
        return this.$store.state.settings.ranSpeedCheck
    }

    get speedCheckResultMbps(): number {
        return this.$store.state.settings.speedCheckResultMbps
    }

    userDisagrees() {
        this.$store.commit('changeLocalRecording', {
            use: !this.$store.state.settings.record.useLocalRecording,
            loc: this.$store.state.settings.record.localRecordingLocation,
            limit: this.$store.state.settings.record.maxLocalRecordingSizeGb
        })
        this.finishSpeedCheck()
    }

    finishSpeedCheck() {
        this.$emit('finishSpeedCheck')
    }

    refreshStatusMessage() {
        this.statusMessage = `${this.funnies.message()} (just kidding)`
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

    async userSpeedCheck() {
        console.log('User speed check to SOV Servers...')
        this.statusMessage = 'Checking your connection speed to SquadOV servers'
        let p = new Promise((resolve, reject) => {
            ipcRenderer.on('finish-user-upload-speed-check', () => {
                resolve(0)
            })
        })

        ipcRenderer.send('user-upload-speed-check')
        return p
    }

    async speedCheckPrompt() {
        // In case the user reverts, the local settings need to be reloaded, or old memory will be used on the settings file
        await this.$store.dispatch('reloadLocalSettings')
        this.showSpeedCheckPrompt = true
        let speedCheckResults = 'enabled'
        if( this.$store.state.settings.record.useLocalRecording ) {
            speedCheckResults = 'disabled'
        }
        this.statusMessage = `We detected your upload speed to our servers is ${this.speedCheckResultMbps} Mb/s. As a result, we've ${speedCheckResults} automatic upload.`
        return new Promise((resolve, reject) => { 
            this.$on('finishSpeedCheck', () => {
                resolve(0)
            })
        }).then(() => {
            // One last SaveLocalSettings call immediately, and ignore InProgress
            saveLocalSettings(this.$store.state.settings, true, true)
        }).finally(() => {
            this.showSpeedCheckPrompt = false
        })
    }

    async requestSessionHeartbeat() {
        this.statusMessage = 'Connecting to SquadOV...'

        // Temporary measure against the white screen
        let intv = window.setTimeout(() => {
            this.showLogout = true
        }, 5000)

        let promise = new Promise((resolve) => {
            ipcRenderer.once('finish-heartbeat', () => {
                resolve(0)
            })
        })
        ipcRenderer.send('start-heartbeat')
        await promise

        window.clearTimeout(intv)
        this.showLogout = false
    }

    async doSetupSequence() {
        // 1. The settings.json needs to be generated properly for any of the next steps to happen.
        await this.generateSettingsFile()

        // 2. Audio device sanity check. We need this primarily to transition the user from an ID-less
        //    settings file (for audio devices) and to backfill in the proper ID based on the given audio device name.
        //    The flip side of doing this will allow us to do an audio device sanity check every time we start up.
        await this.audioDeviceSanityCheck() 

        // 3. Request a session heartbeat. Things after this will require a valid session ID.
        await this.requestSessionHeartbeat()

        // 4. Check if this user has run a speed check on the current computer.
        if(!this.ranSpeedCheck) {
            await this.userSpeedCheck()
            await this.speedCheckPrompt()
        }
        
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