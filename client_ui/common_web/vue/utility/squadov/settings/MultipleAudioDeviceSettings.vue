<template>
    <div>
        <template v-for="(device, index) in internalValue">
            <div :key="index">
                <div class="d-flex align-center">
                    <v-icon small v-if="isInput">
                        mdi-microphone
                    </v-icon>

                    <v-icon small v-else>
                        mdi-headphones
                    </v-icon>

                    <div class="audio-select-div">
                        <v-select
                            v-model="device.device"
                            @input="syncToValue"
                            :items="deviceItems"
                            dense
                            hide-details
                            single-line
                            solo
                        >
                        </v-select>
                    </div>

                    <v-btn icon small @click="refreshDevices">
                        <v-icon>
                            mdi-refresh
                        </v-icon>
                    </v-btn>

                    <v-checkbox
                        class="ma-0"
                        v-model="device.mono"
                        @change="syncToValue"
                        hide-details
                        label="Mono"
                    >
                    </v-checkbox>

                    <v-checkbox
                        v-if="isInput"
                        class="my-0 mr-0 ml-4"
                        v-model="device.voice"
                        @change="syncToValue"
                        hide-details
                        label="Voice"
                    >
                    </v-checkbox>

                    <v-spacer></v-spacer>
                    <v-btn icon color="error" @click="deleteDevice(index)">
                        <v-icon>
                            mdi-delete
                        </v-icon>
                    </v-btn>
                </div>

                <div>
                    <v-slider
                        v-model="device.volume"
                        @input="syncToValue"
                        inverse-label
                        :label="volumeToStr(device.volume)"
                        :min="0.0"
                        :max="2.0"
                        :step="0.01"
                        hide-details
                    >
                    </v-slider>
                </div>
            </div>
        </template>

        <v-btn small text class="mt-1" @click="addNewDevice">
            <v-icon small class="mr-1">
                mdi-plus
            </v-icon>
            Add Device
        </v-btn>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { AudioDeviceSettings, createDefaultAudioDevice } from '@client/js/system/settings'

///#if DESKTOP
import { ipcRenderer } from 'electron'
///#endif 

import { AudioDeviceListingResponse } from '@client/js/system/audio'

@Component
export default class MultipleAudioDeviceSettings extends Vue {
    @Prop({required: true})
    value!: AudioDeviceSettings[]

    @Prop({type: Boolean, default: false})
    isInput!: boolean

    internalValue: AudioDeviceSettings[] = []

    volumeToStr(v: number): string {
        return `${(v * 100.0).toFixed(0)}%`
    }

    addNewDevice() {
        let device = createDefaultAudioDevice()
        device.voice = this.isInput
        this.internalValue.push(device)
        this.syncToValue()
    }
    
    deleteDevice(idx: number) {
        if (idx < 0 || idx >= this.internalValue.length) {
            return
        }
        
        this.internalValue.splice(idx, 1)
        this.syncToValue()
    }

    deviceOptions: string[] = []
    defaultDevice: string = ''
    get deviceItems(): any[] {
        return this.deviceOptions.map((ele: string) => {
            return {
                text: `${ele}${(ele == this.defaultDevice) ? ' [DEFAULT]' : ''}`,
                value: ele,
            }
        })
    }

    refreshDevices() {
///#if DESKTOP
        if (this.isInput) {
            ipcRenderer.send('request-input-devices')
        } else {
            ipcRenderer.send('request-output-devices')
        }
///#endif
    }

    @Watch('value')
    syncFromValue() {
        this.internalValue = JSON.parse(JSON.stringify(this.value))
    }

    syncToValue() {
        this.$emit('input', JSON.parse(JSON.stringify(this.internalValue)))
    }

    mounted() {
///#if DESKTOP
        if (this.isInput) {
            ipcRenderer.on('respond-input-devices', (e: any, resp: AudioDeviceListingResponse) => {
                this.deviceOptions = [
                    'Default Device',
                    ...resp.options
                ]
                this.defaultDevice = resp.default
            })
        } else {
            ipcRenderer.on('respond-output-devices', (e: any, resp: AudioDeviceListingResponse) => {
                this.deviceOptions = [
                    'Default Device',
                    ...resp.options
                ]
                this.defaultDevice = resp.default
            })
        }
///#endif

        this.syncFromValue()
        this.refreshDevices()
    }
}

</script>

<style scoped>

.audio-select-div {
    min-width: 0px;
}

</style>