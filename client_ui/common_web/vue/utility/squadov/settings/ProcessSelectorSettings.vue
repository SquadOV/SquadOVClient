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
                    :items="liveProcesses"
                    height="400"
                    item-height="64"
                >
                    <template v-slot:default="{item}">
                        <div
                            @click="selectedProcess = item"
                        >
                            <live-process-record-display
                                :record="item"
                            >
                            </live-process-record-display>
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
import { Prop } from 'vue-property-decorator'
import { ProcessRecord, LiveProcessRecord } from '@client/js/system/settings'
import LiveProcessRecordDisplay from '@client/vue/utility/system/LiveProcessRecordDisplay.vue'
import ProcessRecordDisplay from '@client/vue/utility/system/ProcessRecordDisplay.vue'

///#if DESKTOP
import { ipcRenderer } from 'electron'
///#endif 

@Component({
    components: {
        ProcessRecordDisplay,
        LiveProcessRecordDisplay,
    }
})
export default class ProcessSelectorSettings extends Vue {
    @Prop({required: true})
    value!: ProcessRecord[]

    internalValue: ProcessRecord[] = []

    showHideProcessSelector: boolean = false

    liveProcesses: LiveProcessRecord[] = []
    selectedProcess: LiveProcessRecord | null = null

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
        this.cancelProcessSelector()
    }

    mounted() {
///#if DESKTOP
        ipcRenderer.on('respond-process-list', (e: any, resp: LiveProcessRecord[]) => {
            this.liveProcesses = resp
        })
///#endif
    }
}

</script>

<style scoped>

.process-select-div {
    min-width: 0px;
}

</style>