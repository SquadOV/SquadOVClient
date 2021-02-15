<template>
    <v-sheet v-if="hasSettings">
        <v-btn
            color="secondary"
            block
            depressed
            text
            tile
            @click="expanded = !expanded"
        >
            <v-icon>
                {{ upDownIcon }}
            </v-icon>

            <v-spacer></v-spacer>

            <div class="d-flex align-center">
                <span :class="`mx-2 dot ${isRecording ? 'recording' : isPaused ? 'paused' : 'available'}`"></span>

                <span class="mx-2 white--text">
                    <span v-if="isRecording">
                        Recording
                    </span>

                    <span v-else-if="isPaused">
                        Paused
                    </span>

                    <span v-else-if="isInGame">
                        In-Game
                    </span>

                    <span v-else>
                        Available
                    </span>
                </span>

                <v-img
                    class="mx-2"
                    v-for="game in gameIconsToShow"
                    :key="`${game}-icon`"
                    contain
                    width="24px"
                    :src="$root.generateAssetUri(`assets/${game}-logo.png`)"
                >
                </v-img>
            </div>

            <v-spacer></v-spacer>

            <v-icon>
                {{ upDownIcon }}
            </v-icon>
        </v-btn>

        <div class="pa-2" v-if="expanded">
            <v-btn
                block
                icon
                tile
                :color="isPaused ? `success`: `warning`"
                @click="togglePause"
                :disabled="isRecording"
            >
                <v-icon v-if="isPaused">
                    mdi-play
                </v-icon>

                <v-icon v-else>
                    mdi-pause
                </v-icon>
            </v-btn>

            <div class="d-flex align-center mt-2">
                <span class="text-overline font-weight-bold mr-4">Quality:</span>
                <v-btn-toggle v-model="res" mandatory rounded dense>
                    <v-btn
                        v-for="res in resolutionItems"
                        :key="`res-${res}`"
                        :value="res"
                    >
                        {{ res }}p
                    </v-btn>
                </v-btn-toggle>
            </div>

            <div class="d-flex align-center mt-2">
                <span class="text-overline font-weight-bold mr-4">FPS:</span>
                <v-btn-toggle v-model="fps" mandatory rounded dense>
                    <v-btn
                        v-for="fps in fpsItems"
                        :key="`fps-${fps}`"
                        :value="fps"
                    >
                        {{ fps }}
                    </v-btn>
                </v-btn-toggle>
            </div>

            <div class="d-flex align-center mt-2">
                <v-icon small>
                    mdi-headphones
                </v-icon>
                <span class="text-overline font-weight-bold ml-1 mr-4">Output:</span>
                <div class="audio-select-div">
                    <v-select
                        v-model="selectedOutput"
                        :items="audioOutputItems"
                        dense
                        hide-details
                        single-line
                        solo
                    >
                    </v-select>
                </div>

                <v-btn icon small @click="refreshAvailableOutputs">
                    <v-icon>
                        mdi-refresh
                    </v-icon>
                </v-btn>
            </div>

            <div>
                <v-slider
                    v-model="outputVolume"
                    inverse-label
                    :label="outputVolumeStr"
                    :min="0.0"
                    :max="2.0"
                    :step="0.01"
                    hide-details
                >
                </v-slider>
            </div>

            <div class="d-flex align-center mt-2">
                <v-icon small>
                    mdi-microphone
                </v-icon>
                <span class="text-overline font-weight-bold ml-1 mr-4">Input:</span>
                <div class="audio-select-div">
                    <v-select
                        v-model="selectedInput"
                        :items="audioInputItems"
                        dense
                        hide-details
                        single-line
                        solo
                    >
                    </v-select>
                </div>

                <v-btn icon small @click="refreshAvailableInputs">
                    <v-icon>
                        mdi-refresh
                    </v-icon>
                </v-btn>
            </div>

            <div>
                <v-slider
                    v-model="inputVolume"
                    inverse-label
                    :label="inputVolumeStr"
                    :min="0.0"
                    :max="2.0"
                    :step="0.01"
                    hide-details
                >
                </v-slider>
            </div>
        </div>
    </v-sheet>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import { ipcRenderer } from 'electron'
import { AudioDeviceListingResponse } from '@client/js/system/audio'

@Component
export default class RecordingStatusWindow extends Vue {
    expanded: boolean = false
    res: number = 1080
    fps: number = 60

    outputOptions: string[] = []
    selectedOutput: string = ''
    defaultOutput: string = ''
    outputVolume: number = 1.0

    inputOptions: string[] = []
    selectedInput: string = ''
    defaultInput: string = ''
    inputVolume: number = 1.0

    get outputVolumeStr(): string {
        return `${(this.outputVolume * 100.0).toFixed(0)}%`
    }

    get inputVolumeStr(): string {
        return `${(this.inputVolume * 100.0).toFixed(0)}%`
    }

    get gameIconsToShow(): string[] {
        if (this.isRecording) {
            return this.$store.state.currentState.recordingGames
        } else {
            return this.$store.state.currentState.runningGames
        }
    }

    get isRecording(): boolean {
        return this.$store.state.currentState.recordingGames.length > 0
    }

    get isInGame(): boolean {
        return this.$store.state.currentState.runningGames.length > 0
    }

    get isPaused(): boolean  {
        return this.$store.state.currentState.paused
    }

    get upDownIcon(): string {
        if (!this.expanded) {
            return 'mdi-chevron-up'
        } else {
            return 'mdi-chevron-down'
        }
    }

    get resolutionItems(): number[] {
        return [
            360,
            480,
            720,
            1080
        ]
    }

    get fpsItems(): number[] {
        return [
            30,
            60
        ]
    }

    get audioOutputItems(): any[] {
        return this.outputOptions.map((ele: string) => {
            return {
                text: `${ele}${(ele == this.defaultOutput) ? ' [DEFAULT]' : ''}`,
                value: ele,
            }
        })
    }

    get audioInputItems(): any[] {
        return this.inputOptions.map((ele: string) => {
            return {
                text: `${ele}${(ele == this.defaultInput) ? ' [DEFAULT]' : ''}`,
                value: ele,
            }
        })
    }

    @Watch('res')
    syncResolution() {
        this.$store.commit('changeRecordSettingRes', this.res)
    }

    @Watch('fps')
    syncFps() {
        this.$store.commit('changeRecordSettingFps', this.fps)
    }

    togglePause() {
        this.$store.commit('toggleRecordingPause')
        ipcRenderer.send('change-state-pause', this.isPaused)
    }

    get hasSettings(): boolean {
        return !!this.$store.state.settings
    }

    @Watch('hasSettings')
    reloadSettings() {
        if (!this.hasSettings) {
            return
        }
        this.res = this.$store.state.settings.record.resY
        this.fps = this.$store.state.settings.record.fps
        this.selectedOutput = this.$store.state.settings.record.outputDevice
        this.outputVolume = this.$store.state.settings.record.outputVolume
        this.selectedInput = this.$store.state.settings.record.inputDevice
        this.inputVolume = this.$store.state.settings.record.inputVolume
    }

    @Watch('selectedOutput')
    @Watch('outputVolume')
    syncOutputDevice() {
        this.$store.commit('changeOutputDevice', {
            device: this.selectedOutput,
            volume: this.outputVolume,
        })
    }


    @Watch('selectedInput')
    @Watch('inputVolume')
    syncInputDevice() {
        this.$store.commit('changeInputDevice', {
            device: this.selectedInput,
            volume: this.inputVolume,
        })
    }
    
    refreshAvailableOutputs() {
        ipcRenderer.send('request-output-devices')
    }

    refreshAvailableInputs() {
        ipcRenderer.send('request-input-devices')
    }

    mounted() {
        this.reloadSettings()
        this.refreshAvailableOutputs()
        this.refreshAvailableInputs()
        ipcRenderer.on('respond-output-devices', (e: any, resp: AudioDeviceListingResponse) => {
            this.outputOptions = [
                'Default Device',
                ...resp.options
            ]

            if (this.selectedOutput == '') {
                this.selectedOutput = 'Default Device'
            }

            this.defaultOutput = resp.default
        })

        ipcRenderer.on('respond-input-devices', (e: any, resp: AudioDeviceListingResponse) => {
            this.inputOptions = [
                'Default Device',
                ...resp.options
            ]

            if (this.selectedInput == '') {
                this.selectedInput = 'Default Device'
            }

            this.defaultInput = resp.default
        })
    }
}

</script>

<style scoped>

.dot {
    height: 24px;
    width: 24px;
    border: 2px solid white;
    border-radius: 50%;
    display: inline-block;
}

.dot.recording {
    animation: 2s linear 0s infinite normal record-pulse;
}

.dot.paused {
    background-color: yellow;
}

.dot.available {
    background-color: green;
}

@keyframes record-pulse {
    0% {
        background-color: #121212;
    }

    50% {
        background-color: red;
    }
}

.audio-select-div {
    max-width: 215px;
}

</style>