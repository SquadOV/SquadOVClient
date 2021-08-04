<template>
    <loading-container :is-loading="!hasSettings">
        <template v-slot:default="{ loading }">
            <v-container fluid>
                <div class="d-flex align-center" v-if="!mini">
                    <span class="text-overline mr-4">Video</span>
                </div>
                <v-divider v-if="!mini"></v-divider>

                <v-row>
                    <v-col cols-sm="12" cols-md="6">
                        <div class="d-flex align-center">
                            <span class="text-overline font-weight-bold mr-4">Quality:</span>
                            <v-btn-toggle :value="resY" @change="changeResY" mandatory rounded dense>
                                <v-btn
                                    v-for="res in resolutionItems"
                                    :key="`res-${res}`"
                                    :value="res"
                                    :disabled="res > $store.state.features.maxRecordPixelY"
                                >
                                    {{ res }}p
                                </v-btn>
                            </v-btn-toggle>
                        </div>
                    </v-col>

                    <v-col cols-sm="12" cols-md="3">
                        <div class="d-flex align-center">
                            <span class="text-overline font-weight-bold mr-4">FPS:</span>
                            <v-btn-toggle :value="fps" @change="changeFps" mandatory rounded dense>
                                <v-btn
                                    v-for="fps in fpsItems"
                                    :key="`fps-${fps}`"
                                    :value="fps"
                                    :disabled="fps > $store.state.features.maxRecordFps"
                                >
                                    {{ fps }}
                                </v-btn>
                            </v-btn-toggle>
                        </div>
                    </v-col>

                    <v-col cols-sm="12" cols-md="3" v-if="!mini">
                        <v-checkbox
                            class="ma-0"
                            :input-value="useVideoHw"
                            @change="changeUseVideoHw"
                            hide-details
                            label="Use GPU Pipeline"
                        >
                            <template v-slot:append>
                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>

                                    Whether to use more of your GPU in the process of feeding captured images to the video encoder.
                                    This feature is not supported on Windows 7.
                                </v-tooltip>
                            </template>
                        </v-checkbox>

                        <v-checkbox
                            class="ma-0"
                            :input-value="useHwEncoder"
                            @change="changeUseHwEncoder"
                            hide-details
                            label="Use GPU Encoder"
                        >
                            <template v-slot:append>
                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>

                                    Whether to use GPU-specific hardware encoders (NVIDIA and AMD) to create the final video.
                                </v-tooltip>
                            </template>
                        </v-checkbox>

                        <v-checkbox
                            class="ma-0"
                            :input-value="useVfr3"
                            @change="changeUseVfr32"
                            hide-details
                            label="Use Variable Framerate"
                        >
                            <template v-slot:append>
                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>

                                    Whether to allow the creation of a video file with variable frame rate.
                                    This may help your videos be smoother if your computer is unable to handle the load of recording at your specified resolution and FPS.
                                </v-tooltip>
                            </template>
                        </v-checkbox>
                    </v-col>
                </v-row>

                <v-row v-if="!mini" align="center">
                    <v-col cols="4">
                        <v-checkbox
                            class="ma-0"
                            :input-value="useBitrate"
                            @change="changeUseBitrate"
                            label="Use Bitrate"
                        >
                            <template v-slot:append>
                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>

                                    If you enable this option, SquadOV will instead try to record at your specified bitrate instead of
                                    trying to encode at a certain (hidden) quality level. This number is capped at 6,000Kbps (kilobits per second) for now.
                                </v-tooltip>
                            </template>
                        </v-checkbox>
                    </v-col>

                    <v-col cols="8">
                        <v-text-field
                            :disabled="!useBitrate"
                            :value="bitrateKbps"
                            @change="changeBitrateKbps(parseInt(arguments[0]))"
                            type="number"
                            solo
                            single-line
                            :rules="bitrateRules"
                        >
                            <template v-slot:append>
                                Kbps
                            </template>
                        </v-text-field>
                    </v-col>
                </v-row>

                <div class="d-flex align-center mt-4" v-if="!mini">
                    <span class="text-overline mr-4">Audio</span>

                    <v-spacer></v-spacer>

                    <v-checkbox
                        class="ma-0"
                        :input-value="$store.state.settings.record.useAudioDriftCompensation"
                        @change="$store.commit('changeAudioDriftCompensation', arguments[0])"
                        label="Use Audio Drift Compensation"
                        hide-details
                        dense
                    >
                        <template v-slot:append>
                            <v-tooltip bottom max-width="450px">
                                <template v-slot:activator="{on, attrs}">
                                    <v-icon v-on="on" v-bind="attrs">
                                        mdi-help-circle
                                    </v-icon>
                                </template>

                                When enabled, SquadOV will attempt to use timestamps to prevent your audio from becoming de-synced.
                                Turn this off if your audio sounds weird.
                            </v-tooltip>
                        </template>
                    </v-checkbox>
                </div>
                <v-divider v-if="!mini"></v-divider>

                <v-row>
                    <v-col cols-sm="12" cols-md="6">
                        <div class="d-flex align-center">
                            <div>
                                <span class="text-overline font-weight-bold">Output Devices</span>
                            </div>
                        </div>

                        <multiple-audio-device-settings
                            :value="$store.state.settings.record.outputDevices"
                            @input="changeOutputDevices"
                        >
                        </multiple-audio-device-settings>
                    </v-col>

                    <v-col cols-sm="12" cols-md="6">
                        <div class="d-flex align-center">
                            <span class="text-overline font-weight-bold">Input Devices</span>
                        </div>

                        <multiple-audio-device-settings
                            :value="$store.state.settings.record.inputDevices"
                            @input="changeInputDevices"
                            is-input
                        >
                        </multiple-audio-device-settings>
                    </v-col>
                </v-row>

                <v-row v-if="!mini && showLocalRecordingSettings">
                    <v-col cols="12">
                        <div class="d-flex align-center">
                            <v-checkbox
                                class="ma-0"
                                :input-value="usePushToTalk"
                                @change="changePushToTalk(arguments[0], pushToTalkKeybind)"
                                hide-details
                                label="Use Push-to-Talk"
                            >
                                <template v-slot:append>
                                    <v-tooltip bottom max-width="450px">
                                        <template v-slot:activator="{on, attrs}">
                                            <v-icon v-on="on" v-bind="attrs">
                                                mdi-help-circle
                                            </v-icon>
                                        </template>

                                        Whether to only record your input device when you have the "push-to-talk" key depressed.
                                    </v-tooltip>
                                </template>
                            </v-checkbox>

                            <v-text-field
                                v-if="usePushToTalk"
                                :value="pttKeybindStr"
                                @keydown="addKeyToPushToTalk"
                                class="ml-8 flex-grow-0"
                                solo
                                single-line
                                hide-details
                                readonly
                                style="width: 500px;"
                            >
                                <template v-slot:append>
                                    <v-btn class="primary" v-if="!pttRecord" @click="startRecordPushToTalk">
                                        Edit Keybind
                                    </v-btn>

                                    <v-btn class="error" @click="stopRecordPushToTalk" v-else>
                                        Stop Recording
                                    </v-btn>
                                </template>
                            </v-text-field>
                        </div>

                        <div class="d-flex align-center">
                            <v-checkbox
                                class="ma-0"
                                :input-value="$store.state.settings.record.useVoiceBasicNoiseFilter"
                                @change="$store.commit('changeVoiceBasicNoiseThreshold', {use: arguments[0], threshold: $store.state.settings.record.voiceFilterThresholdDb})"
                                hide-details
                                label="Use Basic Noise Volume Threshold [Voice]"
                            >
                                <template v-slot:append>
                                    <v-tooltip bottom max-width="450px">
                                        <template v-slot:activator="{on, attrs}">
                                            <v-icon v-on="on" v-bind="attrs">
                                                mdi-help-circle
                                            </v-icon>
                                        </template>

                                        Applies a threshold to your voice inputs.
                                        Only sounds above the specified volume threshold will be recorded.
                                    </v-tooltip>
                                </template>
                            </v-checkbox>

                            <v-slider
                                class="ml-4"
                                :value="$store.state.settings.record.voiceFilterThresholdDb"
                                @input="$store.commit('changeVoiceBasicNoiseThreshold', {use: $store.state.settings.record.useVoiceBasicNoiseFilter, threshold: arguments[0]})"
                                :min="-100"
                                :max="0"
                                :step="1"
                                label="Threshold"
                                color="warning"
                                track-color="success"
                                :disabled="!$store.state.settings.record.useVoiceBasicNoiseFilter"
                                style="max-width: 450px"
                                hide-details
                            >
                            </v-slider>

                            <div class="ml-2 font-weight-bold text-subtitle-2">
                                {{ $store.state.settings.record.voiceFilterThresholdDb }} dB
                            </div>
                        </div>

                        <div class="d-flex align-center">
                            <v-checkbox
                                class="ma-0"
                                :input-value="$store.state.settings.record.useVoiceSpeechNoiseReduction"
                                @change="$store.commit('changeVoiceNoiseSuppression', arguments[0])"
                                hide-details
                                label="Use Advanced Speech Noise Reduction [Voice]"
                            >
                                <template v-slot:append>
                                    <v-tooltip bottom max-width="450px">
                                        <template v-slot:activator="{on, attrs}">
                                            <v-icon v-on="on" v-bind="attrs">
                                                mdi-help-circle
                                            </v-icon>
                                        </template>

                                        Whether to use advanced, super fancy, machine learning algorithms to reduce noise from your voice inputs.
                                        This may be CPU intensive.
                                    </v-tooltip>
                                </template>
                            </v-checkbox>
                        </div>
                    </v-col>
                </v-row>

                <template v-if="!mini && showLocalRecordingSettings">
                    <div class="d-flex align-center mt-4">
                        <span class="text-overline mr-4">Local Storage</span>
                        <v-spacer></v-spacer>

                        <v-tooltip bottom max-width="450px">
                            <template v-slot:activator="{on, attrs}">
                                <v-icon v-on="on" v-bind="attrs">
                                    mdi-help-circle
                                </v-icon>
                            </template>

                            Local storage is used when you have local recording enabled and when downloading match VODs to your computer.
                        </v-tooltip>
                    </div>
                    <v-divider></v-divider>

                    <v-text-field
                        :value="localRecordingLocation"
                        solo
                        hide-details
                        readonly
                    >
                        <template v-slot:prepend>
                            <div>
                                Disk Location: 
                            </div>
                        </template>
                        
                        <template v-slot:append>
                            <div class="d-flex align-center">
                                <v-btn icon @click="onRequestChangeRecordingLocation" :loading="changeLocalRecordingProgress">
                                    <v-icon>
                                        mdi-folder-open
                                    </v-icon>
                                </v-btn>

                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>

                                    Location on disk to store your recorded VODs.
                                </v-tooltip>
                            </div>
                        </template>
                    </v-text-field>

                    <div class="d-flex align-center">
                        <v-checkbox
                            class="ma-0"
                            :input-value="useLocalRecording || !$store.state.features.allowRecordUpload"
                            @change="syncLocalRecording(arguments[0], localRecordingLocation, maxLocalRecordingSizeGb)"
                            hide-details
                            label="Disable Automatic Upload"
                            :disabled="changeLocalRecordingProgress || !$store.state.features.allowRecordUpload"
                        >
                            <template v-slot:append>
                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>

                                    Whether to disable automatic uploading of recorded VODs to SquadOV while you're in-game (i.e. streaming) and enable recording to your local disk instead.
                                    Enabling this will cause your match to only be accessible via the game logs menu and your VODs will not be viewable by anyone else until you upload them.
                                    <b>This is not recommended.</b>
                                </v-tooltip>
                            </template>
                        </v-checkbox>

                        <v-text-field
                            :value="maxLocalRecordingSizeGb"
                            @change="syncLocalRecording(useLocalRecording, localRecordingLocation, parseFloat(arguments[0]))"
                            type="number"
                            solo
                            hide-details
                            class="ml-8"
                            :disabled="changeLocalRecordingProgress"
                        >
                            <template v-slot:prepend>
                                <div>
                                    Max Disk Usage: 
                                </div>
                            </template>
                            <template v-slot:append>
                                <div class="d-flex align-center">
                                    <div>GB</div>
                                    <v-tooltip bottom max-width="450px">
                                        <template v-slot:activator="{on, attrs}">
                                            <v-icon v-on="on" v-bind="attrs">
                                                mdi-help-circle
                                            </v-icon>
                                        </template>

                                        How much space to use for all locally recorded and downloaded VODs.
                                        SquadOV will automatically delete older VODs (determined by when the VOD was recorded or downloaded) when going above this limit. 
                                    </v-tooltip>
                                </div>
                            </template>
                        </v-text-field>

                        <local-disk-space-usage-display
                            :max-width="300"
                            ref="loc"
                        >
                        </local-disk-space-usage-display>

                        <v-btn
                            color="primary"
                            class="ml-2"
                            :to="localManageTo"
                        >
                            Manage
                        </v-btn>
                    </div>
                </template>

                <template v-if="!mini && showLocalRecordingSettings">
                    <div class="d-flex align-center mt-4">
                        <span class="text-overline mr-4">Miscellaneous</span>
                    </div>
                    <v-divider></v-divider>
                    <v-row>
                        <v-col cols="3">
                            <v-text-field
                                :value="vodEndDelaySeconds"
                                @change="changeVodEndDelaySeconds(parseInt(arguments[0]))"
                                type="number"
                                :rules="vodDelayRules"
                                solo
                                single-line
                            >
                                <template v-slot:prepend>
                                    <div>
                                        Post-Game Recording: 
                                    </div>
                                </template>
                                <template v-slot:append>
                                    <div class="d-flex align-center">
                                        <div>Seconds</div>
                                        <v-tooltip bottom max-width="450px">
                                            <template v-slot:activator="{on, attrs}">
                                                <v-icon v-on="on" v-bind="attrs">
                                                    mdi-help-circle
                                                </v-icon>
                                            </template>

                                            Will delay the end of the recorded video by the specified number of seconds so you can capture some post-game video.
                                        </v-tooltip>
                                    </div>
                                </template>
                            </v-text-field>
                        </v-col>
                    </v-row>
                </template>

                <v-snackbar
                    v-model="localRecordChangeFail"
                    :timeout="5000"
                    color="error"
                >
                    Something went wrong changin a local recording setting, please try again!
                </v-snackbar>
            </v-container>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import LocalDiskSpaceUsageDisplay from '@client/vue/utility/squadov/local/LocalDiskSpaceUsageDisplay.vue'
import MultipleAudioDeviceSettings from '@client/vue/utility/squadov/settings/MultipleAudioDeviceSettings.vue'

///#if DESKTOP
import { ipcRenderer } from 'electron'
///#endif 
import { AudioDeviceSettings, changeLocalRecordingSettings } from '@client/js/system/settings'
import { IpcResponse } from '@client/js/system/ipc'
import { LocalStoragePageId } from '@client/js/pages'

@Component({
    components: {
        LoadingContainer,
        LocalDiskSpaceUsageDisplay,
        MultipleAudioDeviceSettings,
    }
})
export default class RecordingSettingsItem extends Vue {
    @Prop({type: Boolean, default: false})
    mini!: boolean

    $refs!: {
        loc: LocalDiskSpaceUsageDisplay
    }

    get localManageTo(): any {
        return {
            name: LocalStoragePageId
        }
    }
    changeOutputDevices(val: AudioDeviceSettings[]) {
        this.$store.commit('changeOutputDevice', val)
    }

    changeInputDevices(val: AudioDeviceSettings[]) {
        this.$store.commit('changeInputDevice', val)
    }

    get resY(): number {
        return this.$store.state.settings.record.resY
    }

    changeResY(val: number) {
        this.$store.commit('changeRecordSettingRes', val)
    }

    get resolutionItems(): number[] {
        return [
            360,
            480,
            720,
            1080
        ]
    }    

    get fps(): number {
        return this.$store.state.settings.record.fps
    }

    changeFps(val: number) {
        this.$store.commit('changeRecordSettingFps', val)
    }

    get fpsItems(): number[] {
        return [
            30,
            60
        ]
    }
    
    get useVideoHw(): boolean {
        return this.$store.state.settings.record.useVideoHw
    }

    changeUseVideoHw(val: boolean) {
        this.$store.commit('changeUseVideoHw', val)
    }

    get useHwEncoder(): boolean {
        return this.$store.state.settings.record.useHwEncoder
    }

    changeUseHwEncoder(val: boolean) {
        this.$store.commit('changeUseHwEncoder', val)
    }

    get useVfr3(): boolean {
        return this.$store.state.settings.record.useVfr3
    }

    changeUseVfr32(val: boolean) {
        this.$store.commit('changeUseVfr32', val)
    }

    get hasSettings(): boolean {
        return !!this.$store.state.settings
    }


    get showLocalRecordingSettings(): boolean {
///#if DESKTOP
        return true
///#else
        return false
///#endif
    }

    onRequestChangeRecordingLocation() {
///#if DESKTOP
        ipcRenderer.invoke('request-user-folder-selection', this.localRecordingLocation).then((folder: string) => {
            this.syncLocalRecording(this.useLocalRecording, folder, this.maxLocalRecordingSizeGb)
        })
///#endif
    }

    get useLocalRecording(): boolean {
        return this.$store.state.settings.record.useLocalRecording
    }

    get localRecordingLocation(): string {
        return this.$store.state.settings.record.localRecordingLocation
    }

    localRecordChangeFail: boolean = false

    get maxLocalRecordingSizeGb(): number {
        return this.$store.state.settings.record.maxLocalRecordingSizeGb
    }

    changeLocalRecordingProgress: boolean = false

    syncLocalRecording(use: boolean, loc: string, limit: number) {
        if (this.changeLocalRecordingProgress) {
            return
        }
        this.changeLocalRecordingProgress = true
        changeLocalRecordingSettings(this.$store.state.settings.record, use, loc, limit).then(() => {
            this.$store.commit('changeLocalRecording', {use, loc, limit})
            this.$refs.loc.refresh()
        }).catch((err: any) => {
            console.log('Failed to change local recording settings: ', err)
            this.localRecordChangeFail = true
        }).finally(() => {
            this.changeLocalRecordingProgress = false
        })
    }

    get useBitrate(): boolean {
        return this.$store.state.settings.record.useBitrate
    }

    changeUseBitrate(v: boolean) {
        this.$store.commit('changeRecordingBitrate', {
            enable: v,
            bitrateKbps: this.bitrateKbps,
        })
    }

    get bitrateKbps(): number {
        return this.$store.state.settings.record.bitrateKbps
    }

    changeBitrateKbps(v: number) {
        this.$store.commit('changeRecordingBitrate', {
            enable: this.useBitrate,
            bitrateKbps: v,
        })
    }

    get bitrateRules(): any[] {
        return [
            (value: string) => !!value || 'Required.',
            (value: string) => !isNaN(parseInt(value)) || 'Must be a number',
            (value: string) => parseInt(value) <= 6000 || 'Must be less than 6000Kbps',
            (value: string) => parseInt(value) > 0 || 'Must be greater than 0Kbps'
        ]
    }

    get vodEndDelaySeconds(): number {
        return this.$store.state.settings.record.vodEndDelaySeconds
    }

    changeVodEndDelaySeconds(v: number) {
        this.$store.commit('changeVodEndDelaySeconds', v)
    }

    get vodDelayRules(): any[] {
        return [
            (value: string) => !!value || 'Required.',
            (value: string) => !isNaN(parseInt(value)) || 'Must be a number',
        ]
    }

    get usePushToTalk(): boolean {
        return this.$store.state.settings.record.usePushToTalk
    }

    get pushToTalkKeybind(): number[] {
        return this.$store.state.settings.keybinds.pushToTalk
    }

    pttRecord: boolean = false
    pttKeybindStr: string = ''
    keybindCache: Map<number, string> = new Map()

    get pttKeySet(): Set<number> {
        return new Set(this.pushToTalkKeybind)
    }

    @Watch('pushToTalkKeybind')
    async resyncPushToTalkStr() {
///#if DESKTOP
        let uncachedCodes = this.pushToTalkKeybind.filter((ele: number) => !this.keybindCache.has(ele))
        for (let code of uncachedCodes) {
            try {
                let k: IpcResponse<string> = await ipcRenderer.invoke('request-key-code-char', code)
                if (k.success) {
                    this.keybindCache.set(code, k.data)
                } else {
                    throw 'Bad keycode?'
                }
            } catch (ex) {
                console.log('Failed to request key code: ', code, ex)
                this.keybindCache.set(code, '<ERROR>')
            }
        }

        this.pttKeybindStr = this.pushToTalkKeybind.map((ele: number) => this.keybindCache.get(ele)!).join(' + ')
///#endif
    }

    mousePttBind: any = null

    startRecordPushToTalk() {
        this.pttRecord = true
        this.changePushToTalk(true, [])

        this.mousePttBind = this.addMouseToPushToTalk.bind(this)
        window.addEventListener('mouseup', this.mousePttBind)
    }

    stopRecordPushToTalk() {
        this.pttRecord = false
        window.removeEventListener('mouseup', this.mousePttBind)
    }

    addVirtualKeyCodeToPushToTalk(code: number) {
        if (this.pttKeySet.has(code)) {
            return
        }
        this.changePushToTalk(this.usePushToTalk, [...this.pushToTalkKeybind, code])
    }

    addKeyToPushToTalk(k: KeyboardEvent) {
        if (!this.pttRecord) {
            return
        }

        this.addVirtualKeyCodeToPushToTalk(k.which)
    }

    addMouseToPushToTalk(m: MouseEvent) {
        if (!this.pttRecord) {
            return
        }

        // This is all windows specific atm.
        switch (m.button) {
            // Mouse wheel -> VK_MBUTTON (0x04)
            case 1:
                this.addVirtualKeyCodeToPushToTalk(0x04)
                break
            // Right click -> VK_RBUTTON (0x02)
            case 2:
                this.addVirtualKeyCodeToPushToTalk(0x02)
                break
            // Browser back button -> VK_XBUTTON1 (0x05)
            case 3:
                this.addVirtualKeyCodeToPushToTalk(0x05)
                break
            // Browser forward button -> VK_XBUTTON2 (0x06)
            case 4:
                this.addVirtualKeyCodeToPushToTalk(0x06)
                break
            // Ignore left click because otherwise it'd capture the end recording button click.
            // Also ignore other buttons as I'm unsure how they're mapped to virtual key codes.
            default:
                return
        }

        m.preventDefault()
    }

    changePushToTalk(use: boolean, keys: number[]) {
        this.$store.commit('changePushToTalk', {
            enable: use,
            ptt: keys,
        })
    }

    refreshFeatureFlags() {
        this.$store.dispatch('loadUserFeatureFlags')
    }

    mounted() {
///#if DESKTOP
        this.resyncPushToTalkStr()
        this.refreshFeatureFlags()
///#endif
    }
}

</script>