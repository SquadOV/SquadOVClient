<template>
    <div class="d-flex flex-column full-width full-parent-height">
        <title-bar no-nav v-if="!preReport"></title-bar>
        <div class="d-flex justify-center align-center flex-column flex-grow-1 pa-4">
            <template v-if="preReport">
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
            </template>

            <template v-else>
                <div class="text-h4 font-weight-bold">
                    {{ reportHeader }}
                </div>

                <div class="text-6">
                    {{ reportSubheader }}
                </div>

                <template v-if="currentStep == ReportStep.Welcome">
                </template>

                <template v-else-if="currentStep == ReportStep.Bandwidth">
                    <div class="text-overline mt-6">
                        Your SquadOV Upload Speed is
                    </div>

                    <div class="text-h3 mt-4">
                        {{ uploadSpeed }}
                    </div>

                    <div class="text-overline mt-4">
                        Mbps
                    </div>

                    <div class="text-subtitle-1 mt-1">
                        You can read more about recording locally on our <a href="#" @click="openUrlInBrowser('https://squadov.zendesk.com/hc/en-us/articles/4497710924059-Recording-videos-locally')">support site</a>.
                        You can make changes to this setting once you're in the app!
                    </div>

                    <div class="mt-1">
                        <v-text-field
                            :value="$store.state.settings.record.localRecordingLocation"
                            solo
                            hide-details
                            readonly
                        >
                            <template v-slot:prepend>
                                <div>
                                    Disk Location: 
                                </div>
                            </template>
                        </v-text-field>
                    </div>
                </template>

                <template v-else-if="currentStep == ReportStep.WowRetail || currentStep == ReportStep.WowTbc || currentStep == ReportStep.WowClassic">
                    <v-container>
                        <v-row>
                            <v-col cols="6">
                                <div class="text-h6">
                                    <span v-if="!!wowReport && !wowReport.installedAddon">SquadOV was unable to automatically configure your WoW setup to its desired configuration.</span>
                                    Please refer to our <a href="#" @click="openUrlInBrowser((currentStep == ReportStep.WowRetail) ? 'https://squadov.zendesk.com/hc/en-us/articles/4505562514715-WoW-Retail-Combat-Log-Addon-Setup' : 'https://squadov.zendesk.com/hc/en-us/articles/4505605365787-WoW-Classic-Combat-Log-Addon-Setup' )">support site</a> to read more on what you need to do.
                                    <span class="font-weight-bold">Do not ignore this. This MUST be setup properly to record.</span>
                                    SquadOV may have automatically performed some changes - you'll want to double check those changes.
                                </div>
                            </v-col>

                            <v-col cols="6" v-if="!!wowReport">
                                <v-list>
                                    <v-list-item v-if="!wowReport.hasClAddon">
                                        <v-list-item-icon>
                                            <v-icon color="error">
                                                mdi-close-circle
                                            </v-icon>
                                        </v-list-item-icon>

                                        <v-list-item-content>
                                            <v-list-item-title>No Combat Log Addon</v-list-item-title>
                                            <v-list-item-subtitle>These addons make sure a combat log gets generated.</v-list-item-subtitle>
                                        </v-list-item-content>
                                    </v-list-item>

                                    <v-list-item v-if="!wowReport.hasRecommendedAddon">
                                        <v-list-item-icon>
                                            <v-icon color="error">
                                                mdi-close-circle
                                            </v-icon>
                                        </v-list-item-icon>

                                        <v-list-item-content>
                                            <v-list-item-title>Recommended Add-On Missing</v-list-item-title>
                                            <v-list-item-subtitle>Please use our recommended combat logger! We know those work with SquadOV for everything we can record.</v-list-item-subtitle>
                                        </v-list-item-content>
                                    </v-list-item>

                                    <v-list-item v-if="wowReport.hasMultipleAddons">
                                        <v-list-item-icon>
                                            <v-icon color="error">
                                                mdi-close-circle
                                            </v-icon>
                                        </v-list-item-icon>

                                        <v-list-item-content>
                                            <v-list-item-title>Multiple Combat Log Addons Detected</v-list-item-title>
                                            <v-list-item-subtitle>Multiple combat log addons can cause the combat log to not be generated.</v-list-item-subtitle>
                                        </v-list-item-content>
                                    </v-list-item>

                                    <v-list-item v-if="wowReport.installedAddon">
                                        <v-list-item-icon>
                                            <v-icon color="success">
                                                mdi-check-circle
                                            </v-icon>
                                        </v-list-item-icon>

                                        <v-list-item-content>
                                            <v-list-item-title>Installed Combat Logger</v-list-item-title>
                                            <v-list-item-subtitle>We made this change for you, make sure it doesn't change!</v-list-item-subtitle>
                                        </v-list-item-content>
                                    </v-list-item>

                                    <v-list-item v-if="wowReport.turnedOnAdvanced">
                                        <v-list-item-icon>
                                            <v-icon color="success">
                                                mdi-check-circle
                                            </v-icon>
                                        </v-list-item-icon>

                                        <v-list-item-content>
                                            <v-list-item-title>Turned on Advanced Combat Logging</v-list-item-title>
                                            <v-list-item-subtitle>We made this change for you, make sure it doesn't change!</v-list-item-subtitle>
                                        </v-list-item-content>
                                    </v-list-item>
                                </v-list>
                            </v-col>
                        </v-row>
                    </v-container>

                    <div class="mt-2 text-subtitle-2" v-if="!!wowReport">
                        You had these combat log addons installed: {{ wowReport.addons.join(', ') }}.
                    </div>
                </template>

                <template v-if="currentStep == ReportStep.Hardware">
                    <div class="text-overline mt-6">
                        What's more important to you?
                    </div>

                    <v-item-group
                        class="full-width"
                        v-model="hwPreference"
                        mandatory
                    >
                        <v-container>
                            <v-row justify="center" align="center">
                                <v-col cols="4">
                                    <v-item v-slot="{ active, toggle }">
                                        <v-card
                                            :color="active ? 'primary' : ''"
                                            @click="toggle"
                                            height="200"
                                        >
                                            <div class="d-flex flex-column justify-center align-center pa-8 full-parent-height">
                                                <div>
                                                    <v-icon x-large>
                                                        mdi-laptop
                                                    </v-icon>
                                                </div>

                                                <div class="mt-2 text-h6">
                                                    In-Game Performance
                                                </div>
                                            </div>
                                        </v-card>
                                    </v-item>
                                </v-col>

                                <v-col cols="4">
                                    <v-item v-slot="{ active, toggle }" >
                                        <v-card
                                            :color="active ? 'primary' : ''"
                                            @click="toggle"
                                            height="200"
                                        >
                                            <div class="d-flex flex-column justify-center align-center pa-8 full-parent-height">
                                                <div>
                                                    <v-icon x-large>
                                                        mdi-video
                                                    </v-icon>
                                                </div>

                                                <div class="mt-2 text-h6">
                                                    Video Quality
                                                </div>
                                            </div>
                                        </v-card>
                                    </v-item>
                                </v-col>

                                <v-col cols="4">
                                    <v-item v-slot="{ active, toggle }" >
                                        <v-card
                                            :color="active ? 'primary' : ''"
                                            @click="toggle"
                                            height="200"
                                        >
                                            <div class="d-flex flex-column justify-center align-center pa-8 full-parent-height">
                                                <div>
                                                    <v-icon x-large>
                                                        mdi-close
                                                    </v-icon>
                                                </div>

                                                <div class="mt-2 text-h6">
                                                    No Change / Default
                                                </div>
                                            </div>
                                        </v-card>
                                    </v-item>
                                </v-col>
                            </v-row>
                        </v-container>
                    </v-item-group>

                    <div class="mt-2 text-subtitle-2" style="max-width: 60%;">
                        We will do our best to maximize the performance of your machine.
                        Please refer to our support site if you do run into issues.
                        If you experience in-game FPS drops, see <a href="#" @click="openUrlInBrowser('https://squadov.zendesk.com/hc/en-us/articles/4503321592731-In-Game-FPS-Drop')">this article</a>.
                        If you experience choppy videos, see <a href="#" @click="openUrlInBrowser('https://squadov.zendesk.com/hc/en-us/articles/4497757709467-Video-FPS-is-choppier-than-expected')">this article</a>.
                    </div>
                </template>

                <v-btn
                    class="mt-4"
                    @click="$emit('next-report-step')"
                    color="primary"
                >
                    {{ (currentStepNum == (totalReportSteps - 1)) ? 'Finish' : 'Next' }}
                </v-btn>

                <v-progress-linear
                    class="mt-4"
                    rounded
                    :value="(currentStepNum + 1) / totalReportSteps * 100"
                    buffer-value="0"
                    stream
                >
                </v-progress-linear>

            </template>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { ipcRenderer } from 'electron'
import { loadLocalSettings } from '@client/js/system/settings'
import { openUrlInBrowser} from '@client/js/external'
const TitleBar = () => import('@client/vue/TitleBar.vue')

///#if DESKTOP
import fs from 'fs'
import path from 'path'
///#endif

import { SquadOvWowRelease, wowReleaseToName } from '@client/js/squadov/game'
import { apiClient } from '@client/js/api'

interface SpeedCheckReport {
    standardUploadMbps: number
}

interface WowSetupReport {
    hasClAddon: boolean
    hasRecommendedAddon: boolean
    hasMultipleAddons: boolean
    installedAddon: boolean
    turnedOnAdvanced: boolean
    addons: string[]
}

interface SetupReport {
    needsAttention: boolean
    firstTime: boolean
    speedCheck: SpeedCheckReport | null
    wowRetail: WowSetupReport | null
    wowClassic: WowSetupReport | null
    wowTbc: WowSetupReport | null
    needsHwCheck: boolean
}

enum ReportStep {
    Welcome,
    Bandwidth,
    Hardware,
    WowRetail,
    WowClassic,
    WowTbc,
}

@Component({
    components: {
        TitleBar
    }
})
export default class SetupProgress extends Vue {
    statusMessage = 'Loading...'
    showLogout: boolean = false
    preReport: boolean = true
    totalReportSteps: number = 0
    reportHeader: string = ''
    reportSubheader: string = ''
    ReportStep = ReportStep
    currentStep: ReportStep = ReportStep.Welcome
    currentStepNum: number = 0
    uploadSpeed: number = 0
    wowReport: WowSetupReport | null = null
    hwPreference: number = 0
    openUrlInBrowser = openUrlInBrowser

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

    async generateSettingsFile() {
        console.log('Generating settings file...')
        this.statusMessage = `Generating settings file...`
        let settings = await loadLocalSettings()
        return settings
    }

    async audioDeviceSanityCheck() {
        console.log('Audio device sanity check...')
        this.statusMessage = `Checking audio devices...`

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

    async presentSetupStartScreen(firstTime: boolean) {
        this.reportHeader = firstTime ? 
            'Welcome to SquadOV!':
            'Welcome back!'
        this.reportSubheader = 'SquadOV has detected a few things that require your attention to ensure that you have the best experience possible! This won\'t take long.'

        this.currentStep = ReportStep.Welcome
        return new Promise((resolve) => {
            this.$on('next-report-step', () => {
                this.currentStepNum += 1
                resolve(0)
            })
        })
    }

    async hardwarePreferenceCheck() {
        this.currentStep = ReportStep.Hardware
        this.reportHeader = 'What\'s your preference?'
        this.reportSubheader = 'SquadOV is a screen recorder and is dependent on your computer being fast enough to run both the game and a recorder simultaneously.'
        return new Promise((resolve) => {
            this.$on('next-report-step', () => {
                if (this.hwPreference == 0) {
                    // In-game Performance
                    this.$store.commit('changeUseHwEncoder', false)
                    this.$store.commit('changeUseVideoHw', false)
                    this.$store.commit('changeRecordSettingRes', 720)
                    this.$store.commit('changeRecordSettingFps', 30)
                } else if (this.hwPreference == 1) {
                    // Video-quality
                    this.$store.commit('changeUseHwEncoder', true)
                    this.$store.commit('changeUseVideoHw', true)
                    this.$store.commit('changeRecordSettingRes', 720)
                    this.$store.commit('changeRecordSettingFps', 60)
                }
                this.currentStepNum += 1
                resolve(0)
            })
        })
    }

    async wowAddonCheck(r: SquadOvWowRelease, report: WowSetupReport) {
        switch (r) {
            case SquadOvWowRelease.Retail:
                this.currentStep = ReportStep.WowRetail
                break
            case SquadOvWowRelease.Tbc:
                this.currentStep = ReportStep.WowTbc
                break
            case SquadOvWowRelease.Vanilla:
                this.currentStep = ReportStep.WowClassic
                break
        }

        this.reportHeader = report.installedAddon ? `WoW ${wowReleaseToName(r)} Add-Ons changed` : `WoW ${wowReleaseToName(r)} May not Record Properly`
        this.reportSubheader = 'Please double check your add-on setup to make sure combat logs are generated properly.'
        this.wowReport = report
        return new Promise((resolve) => {
            this.$on('next-report-step', () => {
                this.currentStepNum += 1
                resolve(0)
            })
        })
    }

    async speedCheckPrompt(r: SpeedCheckReport) {        
        this.currentStep = ReportStep.Bandwidth

        const useLocalRecording = r.standardUploadMbps < 8.0
        this.$store.commit('changeLocalRecording', {
            use: useLocalRecording,
            loc: this.$store.state.settings.record.localRecordingLocation,
            limit: this.$store.state.settings.record.maxLocalRecordingSizeGb
        })

        if (useLocalRecording) {
            this.reportHeader = 'Automatic Upload Disabled'
            this.reportSubheader = 'SquadOV will store recorded videos on your machine rather than upload it to our servers.'
        } else {
            this.reportHeader = 'Automatic Upload Enabled'
            this.reportSubheader = 'SquadOV will upload your recorded videos to our servers instead of storing videos on your machine.'
        }

        this.uploadSpeed = r.standardUploadMbps

        return new Promise((resolve) => {
            this.$on('next-report-step', () => {
                this.currentStepNum += 1
                resolve(0)
            })
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

    async automatedGameSetup() {
        const reportFileName = path.join(process.env.SQUADOV_USER_APP_FOLDER!, '.agdata')
        if (fs.existsSync(reportFileName)) {
            fs.rmSync(reportFileName)
        }

        console.log('Doing automated game setup: ', reportFileName)
        this.statusMessage = 'Checking SquadOV Settings...'

        let p = new Promise<SetupReport | null>((resolve, reject) => {
            ipcRenderer.on('finish-automated-game-setup', () => {
                if (fs.existsSync(reportFileName)) {
                    resolve(JSON.parse(fs.readFileSync(reportFileName, 'utf-8')))
                } else {
                    resolve(null)
                }
            })
        })

        // This sends a request to generate the setup report to the JSON file we specify.
        // This way we know what options we need to present to the user in terms of getting
        // them setup further.
        ipcRenderer.send('request-automated-game-setup', reportFileName)
        return p
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

        // 4. Automated game setup
        let setupReport: SetupReport | null = await this.automatedGameSetup()
        if (!setupReport) {
            console.error('Failed to generate setup report')
        } else if (setupReport.needsAttention) {
            this.preReport = false

            apiClient.markUserAnalyticsEvent('startSetupReport', 'DESKTOP').catch((err: any) => {
                console.warn('Failed to mark start setup report: ', err)
            })
            
            // Switch to the larger setup screen.
            ipcRenderer.send('setup-resize-window')

            this.totalReportSteps = 1 + (!!setupReport.speedCheck ? 1 : 0) +
                (!!setupReport.wowRetail ? 1 : 0) + 
                (!!setupReport.wowClassic ? 1: 0) +
                (!!setupReport.wowTbc ? 1: 0) +
                (setupReport.needsHwCheck ? 1 : 0)
            this.currentStepNum = 0
            await this.presentSetupStartScreen(setupReport.firstTime)

            // If we had to test the user's internet speed, present that choice to the user.
            if (!!setupReport.speedCheck) {
                await this.speedCheckPrompt(setupReport.speedCheck)
            }

            // For WoW - need to make sure the user's combat log addon is setup properly.
            if (!!setupReport.wowRetail) {
                await this.wowAddonCheck(SquadOvWowRelease.Retail, setupReport.wowRetail)
            }

            if (!!setupReport.wowClassic) {
                await this.wowAddonCheck(SquadOvWowRelease.Vanilla, setupReport.wowClassic)
            }

            if (!!setupReport.wowTbc) {
                await this.wowAddonCheck(SquadOvWowRelease.Tbc, setupReport.wowTbc)
            }

            // Need to make the user decide between whether they want video quality or in-game FPS.
            if (setupReport.needsHwCheck) {
                await this.hardwarePreferenceCheck()
            }

            apiClient.markUserAnalyticsEvent('finishSetupReport', 'DESKTOP').catch((err: any) => {
                console.warn('Failed to mark finish setup report: ', err)
            })
            this.preReport = true
        }
        
        ipcRenderer.send('finish-setup')
    }

    mounted() {
        this.doSetupSequence()
    }
}

</script>

<style scoped>

</style>