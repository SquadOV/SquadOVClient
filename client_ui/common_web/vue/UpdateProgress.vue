<template>
    <div class="update-div d-flex justify-center align-center flex-column">
        <template v-if="!hasUpdate">
            <v-progress-circular class="mb-4" indeterminate size="64">
            </v-progress-circular>

            <div >
                SquadOV is checking for updates...
            </div>
        </template>

        <template v-else>
            <v-progress-linear class="mb-4" :value="downloadProgress">
            </v-progress-linear>

            <div>
                Downloading SquadOV v{{ updateVersion }} ... {{ displayPercent }}
            </div>
        </template>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { ipcRenderer } from 'electron'
import { UpdateInfo } from 'electron-updater'
import { ProgressInfo } from 'electron-builder'

@Component
export default class UpdateProgress extends Vue {
    hasUpdate: boolean = false
    updateVersion: string = ''
    downloadProgress: number = 0

    get displayPercent() : string {
        return `${Math.floor(this.downloadProgress)}%`
    }

    mounted() {
        ipcRenderer.on('update-update-available', (e: any, info: UpdateInfo) => {
            this.hasUpdate = true
            this.updateVersion = info.version
        })

        ipcRenderer.on('update-download-progress', (e: any, info: ProgressInfo) => {
            console.log('Auto-Update Download Progress: ', info)
            this.downloadProgress = info.percent
            this.$forceUpdate()
        })
    }
}

</script>

<style scoped>

.update-div {
    width: 300px;
    height: 300px;
}

</style>