<template>
    <div class="d-flex flex-column align-center justify-center" v-if="uploadProgress !== null">
        <v-progress-circular size="16" indeterminate></v-progress-circular>

        <div>
            {{ (uploadProgress * 100.0).toFixed(0) }}% 
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { DownloadUploadProgressCb, manager } from '@client/js/vods/local_manager'

@Component
export default class UploadProgressDisplay extends Vue {
    @Prop({required: true})
    videoUuid!: string

    uploadProgress: number | null = null
    registeredUploadCb: DownloadUploadProgressCb | null = null

    get isUploading(): boolean {
        return this.uploadProgress !== null
    }

    onUploadProgress(progress: number, finish: boolean, error: boolean) {
        if (finish) {
            this.uploadProgress = null
        } else {
            this.uploadProgress = progress
        }
    }

    mounted() {
        this.registerCallbacks()
    }

    @Watch('videoUuid')
    registerCallbacks() {
        this.unregisterCallbacks()

        this.registeredUploadCb = this.onUploadProgress.bind(this)
        manager.uploads.addCallback(this.videoUuid, this.registeredUploadCb!)
    }

    unregisterCallbacks() {
        if (!!this.registeredUploadCb) {
            manager.uploads.removeCallback(this.videoUuid, this.registeredUploadCb)
        }
        this.registeredUploadCb = null
    }

    beforeDestroy() {
        this.unregisterCallbacks()
    }
}

</script>