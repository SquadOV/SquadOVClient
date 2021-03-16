<template>
    <loading-container :is-loading="!hasSettings">
        <template v-slot:default="{ loading }">
            <v-container fluid>
                <div class="d-flex align-center" v-if="!mini">
                    <span class="text-overline mr-4">Video</span>
                    <v-divider></v-divider>
                </div>

                <v-row>
                    <v-col cols-sm="12" cols-md="6">
                        <div class="d-flex align-center">
                            <span class="text-overline font-weight-bold mr-4">Quality:</span>
                            <v-btn-toggle :value="resY" @change="changeResY" mandatory rounded dense>
                                <v-btn
                                    v-for="res in resolutionItems"
                                    :key="`res-${res}`"
                                    :value="res"
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
                            label="Use GPU Encoder"
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
                            label="Use GPU Pipeline"
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
                    </v-col>
                </v-row>

                <div class="d-flex align-center mt-4" v-if="!mini">
                    <span class="text-overline mr-4">Audio</span>
                    <v-divider></v-divider>
                </div>

                <v-row>
                    <v-col cols-sm="12" cols-md="6">
                        <div class="d-flex align-center">
                            <div>
                                <v-icon small>
                                    mdi-headphones
                                </v-icon>
                            </div>

                            <div>
                                <span class="text-overline font-weight-bold ml-1 mr-4">Output:</span>
                            </div>
                            <div class="audio-select-div">
                                <v-select
                                    :value="selectedOutput"
                                    @input="changeSelectedOutput"
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
                                :value="outputVolume"
                                @input="changeOutputVolume"
                                inverse-label
                                :label="outputVolumeStr"
                                :min="0.0"
                                :max="2.0"
                                :step="0.01"
                                hide-details
                            >
                            </v-slider>
                        </div>
                    </v-col>
                    <v-col cols-sm="12" cols-md="6">
                        <div class="d-flex align-center">
                            <v-icon small>
                                mdi-microphone
                            </v-icon>
                            <span class="text-overline font-weight-bold ml-1 mr-4">Input:</span>
                            <div class="audio-select-div">
                                <v-select
                                    :value="selectedInput"
                                    @input="changeSelectedInput"
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
                                :value="inputVolume"
                                @input="changeInputVolume"
                                inverse-label
                                :label="inputVolumeStr"
                                :min="0.0"
                                :max="2.0"
                                :step="0.01"
                                hide-details
                            >
                            </v-slider>
                        </div>
                    </v-col>
                </v-row>
            </v-container>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'

///#if DESKTOP
import { ipcRenderer } from 'electron'
///#endif 
import { AudioDeviceListingResponse } from '@client/js/system/audio'

@Component({
    components: {
        LoadingContainer,
    }
})
export default class RecordingSettingsItem extends Vue {
    @Prop({type: Boolean, default: false})
    mini!: boolean

    outputOptions: string[] = []
    defaultOutput: string = ''
    get audioOutputItems(): any[] {
        return this.outputOptions.map((ele: string) => {
            return {
                text: `${ele}${(ele == this.defaultOutput) ? ' [DEFAULT]' : ''}`,
                value: ele,
            }
        })
    }

    refreshAvailableOutputs() {
///#if DESKTOP
        ipcRenderer.send('request-output-devices')
///#endif
    }

    inputOptions: string[] = []
    defaultInput: string = ''
    get audioInputItems(): any[] {
        return this.inputOptions.map((ele: string) => {
            return {
                text: `${ele}${(ele == this.defaultInput) ? ' [DEFAULT]' : ''}`,
                value: ele,
            }
        })
    }

    refreshAvailableInputs() {
///#if DESKTOP
        ipcRenderer.send('request-input-devices')
///#endif
    }

    get selectedOutput(): string {
        return this.$store.state.settings.record.outputDevice
    }

    changeSelectedOutput(val: string) {
        this.$store.commit('changeOutputDevice', {
            device: val,
            volume: this.outputVolume,
        })
    }

    get outputVolume(): number {
        return this.$store.state.settings.record.outputVolume
    }

    changeOutputVolume(val: number) {
        this.$store.commit('changeOutputDevice', {
            device: this.selectedOutput,
            volume: val,
        })
    }

    get outputVolumeStr(): string {
        return `${(this.outputVolume * 100.0).toFixed(0)}%`
    }

    get selectedInput(): string {
        return this.$store.state.settings.record.inputDevice
    }

    changeSelectedInput(val: string) {
        this.$store.commit('changeInputDevice', {
            device: val,
            volume: this.inputVolume,
        })
    }

    get inputVolume(): number {
        return this.$store.state.settings.record.inputVolume
    }

    changeInputVolume(val: number) {
        this.$store.commit('changeInputDevice', {
            device: this.selectedInput,
            volume: val,
        })
    }
    
    get inputVolumeStr(): string {
        return `${(this.inputVolume * 100.0).toFixed(0)}%`
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

    get hasSettings(): boolean {
        return !!this.$store.state.settings
    }

    mounted() {
///#if DESKTOP        
        this.refreshAvailableOutputs()
        this.refreshAvailableInputs()

        ipcRenderer.on('respond-output-devices', (e: any, resp: AudioDeviceListingResponse) => {
            this.outputOptions = [
                'Default Device',
                ...resp.options
            ]

            if (this.selectedOutput == '') {
                this.changeSelectedOutput('Default Device')
            }

            this.defaultOutput = resp.default
        })

        ipcRenderer.on('respond-input-devices', (e: any, resp: AudioDeviceListingResponse) => {
            this.inputOptions = [
                'Default Device',
                ...resp.options
            ]

            if (this.selectedInput == '') {
                this.changeSelectedInput('Default Device')
            }

            this.defaultInput = resp.default
        })
///#endif
    }
}

</script>

<style scoped>

.audio-select-div {
    min-width: 0;
}

</style>