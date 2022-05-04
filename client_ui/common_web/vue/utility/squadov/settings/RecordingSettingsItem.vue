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
                            <span class="text-overline font-weight-bold mr-4">Resolution:</span>
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

                        <div class="d-flex align-center mt-4">
                            <span class="text-overline font-weight-bold mr-4">Quality:</span>
                            <v-select
                                dense
                                :value="quality"
                                @input="changeQuality"
                                :items="qualityItems"
                                hide-details=""
                                outlined
                            >
                            </v-select>

                            <v-checkbox
                                class="ma-0"
                                :input-value="$store.state.settings.record.useCbr"
                                @change="$store.commit('changeUseCbr', arguments[0])"
                                hide-details
                                label="Use CBR"
                                v-if="!mini"
                            >
                                <template v-slot:append>
                                    <v-tooltip bottom max-width="450px">
                                        <template v-slot:activator="{on, attrs}">
                                            <v-icon v-on="on" v-bind="attrs">
                                                mdi-help-circle
                                            </v-icon>
                                        </template>

                                        Whether to use a constant (instead a variable one) bitrate for your videos. This may result in higher quality and larger videos.
                                    </v-tooltip>
                                </template>
                            </v-checkbox>
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
                            :input-value="$store.state.settings.record.useVideoHw2"
                            @change="$store.commit('changeUseVideoHw', arguments[0])"
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
                            :input-value="$store.state.settings.record.useHwEncoder"
                            @change="$store.commit('changeUseHwEncoder', arguments[0])"
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
                            :input-value="$store.state.settings.record.useNativeAspectRatio"
                            @change="$store.commit('changeUseNativeAspectRatio', arguments[0])"
                            hide-details
                            label="Use Native Aspect Ratio"
                        >
                            <template v-slot:append>
                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>

                                    If you play on a stretched resolution, use this to force SquadOV to record a video with your native aspect ratio rather than the one specified in the game.
                                    It is not recommend you turn this option on if you do not play with a stretched resolution.
                                </v-tooltip>
                            </template>
                        </v-checkbox>
                    </v-col>

                    <v-col cols-sm="12" cols-md="3" v-if="!mini">
                        <v-checkbox
                            class="ma-0"
                            :input-value="$store.state.settings.record.useVfr4"
                            @change="$store.commit('changeUseVfr', arguments[0])"
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

                        <v-checkbox
                            class="ma-0"
                            :input-value="$store.state.settings.record.recordMouse2"
                            @change="$store.commit('changeRecordMouse', arguments[0])"
                            hide-details
                            label="Record Cursor (Full-Screen only)"
                        >
                            <template v-slot:append>
                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>
                                    When recording in full-screen mode, whether to also capture your cursor.
                                </v-tooltip>
                            </template>
                        </v-checkbox>
                    </v-col>
                </v-row>

                <div class="d-flex align-center mt-4" v-if="!mini">
                    <span class="text-overline mr-4">Audio</span>

                    <v-spacer></v-spacer>

                    <v-checkbox
                        class="my-0 ml-0 mr-4"
                        :input-value="$store.state.settings.record.useWASAPIRecording3"
                        @change="$store.commit('changeUseWASAPIRecording', arguments[0])"
                        label="Use Native Recording"
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

                                When enabled, SquadOV will interface directly with native APIs to record audio.
                                This is necessary for filtering audio by program, and may be more reliable in recording.
                            </v-tooltip>
                        </template>
                    </v-checkbox>

                    <v-btn
                        class="my-0 ml-0 mr-4"
                        v-if="$store.state.settings.record.useWASAPIRecording3 && !$store.state.settings.record.perProcessRecordingOsCheck"
                        color="warning"
                        dense
                        @click="showHideOsVerification = true"
                    >
                        Enable Per-Process Recording Support
                    </v-btn>

                    <v-checkbox
                        class="my-0 ml-0 mr-4"
                        v-if="$store.state.settings.record.useWASAPIRecording3"
                        :input-value="$store.state.settings.record.usePerProcessRecording"
                        :disabled="!$store.state.settings.record.perProcessRecordingOsCheck"
                        @change="$store.commit('changeUseProcessAudioRecording', {enable: arguments[0], game: $store.state.settings.record.recordGameAudio, processes: $store.state.settings.record.processesToRecord})"
                        label="Use Per-Process Recording"
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

                                When enabled, SquadOV will attempt to record audio directly from the specified processes.
                                This will only work on Windows 11 and newer versions of Windows 10.
                            </v-tooltip>
                        </template>
                    </v-checkbox>

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
                <v-divider class="mt-2" v-if="!mini"></v-divider>

                <v-row>
                    <v-col cols-sm="12" cols-md="6">
                        <template v-if="!$store.state.settings.record.usePerProcessRecording || !$store.state.settings.record.useWASAPIRecording3">
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
                        </template>

                        <template v-else>
                            <div class="d-flex align-center">
                                <div>
                                    <span class="text-overline font-weight-bold">Processes to Record</span>
                                </div>
                            </div>

                            <v-checkbox
                                class="ma-0"
                                :input-value="$store.state.settings.record.recordGameAudio"
                                @change="$store.commit('changeUseProcessAudioRecording', {enable: $store.state.settings.record.usePerProcessRecording, game: arguments[0], processes: $store.state.settings.record.processesToRecord})"
                                label="Record Game Audio"
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

                                        Whether to record audio from the game being recorded.
                                    </v-tooltip>
                                </template>
                            </v-checkbox>

                            <div>
                                <v-slider
                                    :value="$store.state.settings.record.gameAudioVolume"
                                    @input="$store.commit('changeGameAudioVolume', arguments[0])"
                                    inverse-label
                                    :label="volumeToStr($store.state.settings.record.gameAudioVolume)"
                                    :min="0.0"
                                    :max="2.0"
                                    :step="0.01"
                                    hide-details
                                >
                                    <template v-slot:prepend>
                                        Game Volume: 
                                    </template>
                                </v-slider>
                            </div>

                            <process-selector-settings
                                :value="$store.state.settings.record.processesToRecord"
                                @input="$store.commit('changeUseProcessAudioRecording', {enable: $store.state.settings.record.usePerProcessRecording, game: $store.state.settings.record.recordGameAudio, processes: arguments[0]})"
                            >
                            </process-selector-settings>
                        </template>
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
                                @change="changePushToTalk(arguments[0], $store.state.settings.keybinds2.pushToTalk.keys, $store.state.settings.keybinds2.pushToTalk2.keys)"
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

                            <template v-if="usePushToTalk">
                                <keybind-settings-item
                                    :value="$store.state.settings.keybinds2.pushToTalk"
                                    @input="changePushToTalk(usePushToTalk, arguments[0].keys, $store.state.settings.keybinds2.pushToTalk2.keys)"
                                >
                                </keybind-settings-item>

                                <keybind-settings-item
                                    :value="$store.state.settings.keybinds2.pushToTalk2"
                                    @input="changePushToTalk(usePushToTalk, $store.state.settings.keybinds2.pushToTalk.keys, arguments[0].keys)"
                                >
                                </keybind-settings-item>
                            </template>
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
                    Something went wrong changing a local recording setting. You can not change the local recording folder to a root directory or a non-empty folder.
                </v-snackbar>

                <v-dialog
                    v-model="showHideOsVerification"
                    persistent
                    max-width="40%"
                >
                    <v-card>
                        <v-card-title>
                            Are you sure your operating system supports this feature?
                        </v-card-title>
                        <v-divider></v-divider>

                        <div class="ma-4">
                            Per-process recording is only available on <span class="font-weight-bold">Windows 11</span> and on certain versions (<span class="font-weight-bold">21H1, 21H2</span>) of Windows 10.
                            DO NOT ENABLE THIS FEATURE UNLESS YOU ARE SURE THAT YOUR COMPUTER IS FEATURE EQUIVALENT WITH THESE OPERATING SYSTEMS.
                        </div>

                        <v-card-actions>
                            <v-btn
                            color="error"
                            @click="showHideOsVerification = false"
                        >
                            Nevermind!
                        </v-btn>

                        <v-spacer></v-spacer>

                        <v-btn
                            color="success"
                            @click="confirmOsVerification"
                        >
                            Confirm
                        </v-btn>
                        </v-card-actions>
                    </v-card>
                </v-dialog>
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
import ProcessSelectorSettings from '@client/vue/utility/squadov/settings/ProcessSelectorSettings.vue'

///#if DESKTOP
import { ipcRenderer } from 'electron'
///#endif 
import { AudioDeviceSettings, changeLocalRecordingSettings } from '@client/js/system/settings'
import { IpcResponse } from '@client/js/system/ipc'
import { LocalStoragePageId } from '@client/js/pages'
import KeybindSettingsItem from '@client/vue/utility/squadov/settings/KeybindSettingsItem.vue'

@Component({
    components: {
        LoadingContainer,
        LocalDiskSpaceUsageDisplay,
        MultipleAudioDeviceSettings,
        ProcessSelectorSettings,
        KeybindSettingsItem,
    }
})
export default class RecordingSettingsItem extends Vue {
    @Prop({type: Boolean, default: false})
    mini!: boolean

    showHideOsVerification: boolean = false

    $refs!: {
        loc: LocalDiskSpaceUsageDisplay
    }

    confirmOsVerification() {
        this.showHideOsVerification = false
        this.$store.commit('changePerProcessRecordingOsCheck', true)
    }

    volumeToStr(v: number): string {
        return `${(v * 100.0).toFixed(0)}%`
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

    get quality(): number {
        return this.$store.state.settings.record.bitrateKbps
    }

    changeQuality(val: number) {
        this.$store.commit('changeRecordSettingQuality', val)
    }

    get qualityItems(): any[] {
        return [
            {
                text: 'Ultra-High (9000 kbps)',
                value: 9000,
            },
            {
                text: 'High (6000 kbps)',
                value: 6000,
            },
            {
                text: 'Medium (4000 kbps)',
                value: 4000,
            },
            {
                text: 'Low (2000 kbps)',
                value: 2000,
            },
            {
                text: 'Very Low (1000 kbps)',
                value: 1000,
            },
        ].map((ele) => {
            return {
                disabled: (ele.value > this.$store.state.features.maxBitrateKbps),
                ...ele
            }
        })
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
            console.error('Failed to change local recording settings: ', err)
            this.localRecordChangeFail = true
        }).finally(() => {
            this.changeLocalRecordingProgress = false
        })
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

    changePushToTalk(use: boolean, keys: number[], alt: number[]) {
        this.$store.commit('changePushToTalk', {
            enable: use,
            ptt: keys,
            altPtt: alt,
        })
    }

    refreshFeatureFlags() {
        this.$store.dispatch('loadUserFeatureFlags')
    }

    mounted() {
///#if DESKTOP
        this.refreshFeatureFlags()
///#endif
    }
}

</script>