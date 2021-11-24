<template>
    <div>
        <v-dialog
            v-model="showHideProcessSelector"
            width="60%"
            persistent
        >
            <v-card>
                <v-card-title>
                    Select Process
                </v-card-title>
                <v-divider></v-divider>
                
                <v-virtual-scroll
                    class="ma-4"
                    :items="liveProcesses"
                    height="400"
                    item-height="32"
                >
                    <template v-slot:default="{item}">
                        <div
                            :class="`selector ${(!!selectedProcess && selectedProcess.exe === item.exe) ? 'active-selector' : ''}`"
                            @click="selectedProcess = item"
                        >
                            <process-record-display
                                :record="item"
                            >
                            </process-record-display>
                        </div>
                    </template>
                </v-virtual-scroll>

                <v-card-actions>
                    <v-btn
                        color="error"
                        @click="cancelProcessSelector"
                    >
                        Cancel
                    </v-btn>

                    <v-spacer></v-spacer>

                    <v-btn
                        color="success"
                        @click="selectProcess"
                        :disabled="!selectedProcess"
                    >
                        Select
                    </v-btn>
                </v-card-actions>
            </v-card>
        </v-dialog>

        <template v-for="(device, index) in internalValue">
            <div :key="index">
            </div>
        </template>

        <v-btn small text class="mt-1" @click="startAddNewProcess">
            <v-icon small class="mr-1">
                mdi-plus
            </v-icon>
            Add Process
        </v-btn>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { ProcessRecord } from '@client/js/system/settings'
import ProcessRecordDisplay from '@client/vue/utility/system/ProcessRecordDisplay.vue'

///#if DESKTOP
import { ipcRenderer } from 'electron'
///#endif 

@Component({
    components: {
        ProcessRecordDisplay,
    }
})
export default class ProcessSelectorSettings extends Vue {
    @Prop({required: true})
    value!: ProcessRecord[]

    internalValue: ProcessRecord[] = []

    showHideProcessSelector: boolean = false

    liveProcesses: ProcessRecord[] = []
    selectedProcess: ProcessRecord | null = null

    refreshProcesses() {
///#if DESKTOP
        ipcRenderer.send('request-process-list')
///#endif 

        if (this.showHideProcessSelector) {
            setTimeout(() => {
                this.refreshProcesses()
            }, 5000)
        }
    }

    cancelProcessSelector() {
        this.showHideProcessSelector = false
    }

    startAddNewProcess() {
        this.selectedProcess = null
        this.showHideProcessSelector = true
        this.refreshProcesses()
    }

    selectProcess() {
        if (!!this.selectedProcess) {
            this.internalValue.push({
                name: this.selectedProcess.name,
                exe: this.selectedProcess.exe,
                ico: this.selectedProcess.ico,
            })
        }

        this.syncToValue()
        this.cancelProcessSelector()
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
        ipcRenderer.on('respond-process-list', (e: any, resp: ProcessRecord[]) => {
            this.liveProcesses = resp.sort((a: ProcessRecord, b: ProcessRecord) => {
                let aLower = a.name.toLowerCase()
                let bLower = b.name.toLowerCase()
                if (aLower < bLower) {
                    return -1
                } else if (aLower > bLower) {
                    return 1
                } else {
                    return 0
                }
            })
        })
///#endif

        this.syncFromValue()
    }
}

</script>

<style scoped>

.process-select-div {
    min-width: 0px;
}

.selector {
    cursor: pointer;
}

.selector:hover {
    background-color: #919191;
}

.active-selector {
    background-color: #919191;
}

</style>