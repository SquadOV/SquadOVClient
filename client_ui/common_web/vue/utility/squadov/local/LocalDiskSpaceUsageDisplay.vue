<template>
    <div class="d-flex">
        <div :style="barStyle">
            <v-progress-linear
                :color="diskSpaceUsageColor"
                :value="localDiskSpaceRecordUsageGb / maxLocalRecordingSizeGb * 100"
                striped
                rounded
                height="24"
            >
            </v-progress-linear>
        </div>

        <div class="ml-4">
            {{ localDiskSpaceRecordUsageGb.toFixed(2) }} / {{ maxLocalRecordingSizeGb.toFixed(2) }} GB
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { computeFileFolderSizeGb } from '@client/js/system/settings'

@Component
export default class LocalDiskSpaceUsageDisplay extends Vue {
    @Prop({default: 300})
    maxBarWidth!: number

    get barStyle(): any {
        return {
            'max-width': `${this.maxBarWidth}px`,
            'width': `${this.maxBarWidth}px`
        }
    }

    get localRecordingLocation(): string {
        return this.$store.state.settings.record.localRecordingLocation
    }

    localDiskSpaceRecordUsageGb: number = 0

    @Watch('localRecordingLocation')
    refreshLocalDiskSpaceRecordUsage() {
        computeFileFolderSizeGb(this.localRecordingLocation).then((resp: number) => {
            this.localDiskSpaceRecordUsageGb = resp
        }).catch((err: any) => {
            console.error('Failed to get local disk space record usage: ', err)
        })
    }

    get maxLocalRecordingSizeGb(): number {
        return this.$store.state.settings.record.maxLocalRecordingSizeGb
    }

    get diskSpaceUsageColor(): string {
        let percent = this.localDiskSpaceRecordUsageGb / this.maxLocalRecordingSizeGb
        if (percent < 0.5) {
            return 'success'
        } else if (percent < 0.9) {
            return 'warning'
        } else {
            return 'error'
        }
    }

    mounted() {
        this.refresh()
    }

    refresh() {
        this.refreshLocalDiskSpaceRecordUsage()
    }
}


</script>