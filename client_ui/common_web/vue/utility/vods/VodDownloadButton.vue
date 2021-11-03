<template>
    <div>
        <v-tooltip bottom v-if="hasLocal">
            <template v-slot:activator="{on, attrs}">
                <v-btn color="warning icon" icon :loading="checkingForLocal" v-on="on" v-bind="attrs" @click="openLocalDownload" >
                    <v-icon>
                        mdi-folder-open
                    </v-icon>
                </v-btn>
            </template>

            Open Folder Location
        </v-tooltip>

        <v-tooltip bottom v-else-if="!!downloadUri && useSimpleDownload && hasFastify" >
            <template v-slot:activator="{on, attrs}">
                <v-btn color="warning" icon :href="downloadUri" :loading="checkingForLocal" v-on="on" v-bind="attrs">
                    <v-icon>
                        mdi-download
                    </v-icon>
                </v-btn>
            </template>

            Download
        </v-tooltip>

        <v-tooltip bottom v-else-if="!useSimpleDownload && hasFastify">
            <template v-slot:activator="{on, attrs}">
                <v-btn color="warning" icon @click="doLocalDownload" :loading="downloadProgress !== null || checkingForLocal" v-on="on" v-bind="attrs">
                    <v-icon>
                        mdi-download
                    </v-icon>
                </v-btn>
            </template>

            Download
        </v-tooltip>

        <div v-if="downloadProgress !== null">
            {{ (downloadProgress * 100.0).toFixed(0) }}% 
        </div>

        <v-snackbar
            v-model="downloadError"
            :timeout="5000"
            color="error"
        >
            Failed to download the VOD, please try again later.
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Component, {mixins} from 'vue-class-component'
import CommonComponent from '@client/vue/CommonComponent'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import * as vod from '@client/js/squadov/vod'
import { DownloadUploadProgressCb, manager } from '@client/js/vods/local_manager'

/// #if DESKTOP
import { shell, ipcRenderer } from 'electron'
import { IpcResponse } from '@client/js/system/ipc'
/// #endif

@Component
export default class VodDownloadButton extends mixins(CommonComponent) {
    @Prop({ required: true })
    videoUuid!: string

    @Prop({ required: true})
    track!: vod.VodTrack | null

    localVodLocation: string | null = null
    downloadProgress: number | null = null
    downloadError: boolean = false
    checkingForLocal: boolean = false
    downloadUri: string | null = null

    _registeredDownloadCb: DownloadUploadProgressCb | null = null

    get hasLocal(): boolean {
        return !!this.localVodLocation
    }

    get useSimpleDownload(): boolean {
///#if DESKTOP
        return false
///#else
        return true
///#endif
    }

    get hasFastify(): boolean {
        if (!this.track) {
            return false
        }
        return this.track.segments[0].mimeType !== 'video/mp2t'
    }

    @Watch('videoUuid')
    recheckForLocalFile() {
        if (this.checkingForLocal) {
            return
        }

///#if DESKTOP
        this.checkingForLocal = true
        ipcRenderer.invoke('check-vod-local', this.videoUuid).then((resp: IpcResponse<string>) => {
            if (resp.success) {
                this.localVodLocation = resp.data
            } else {
                this.localVodLocation = null
            }
        }).catch((err: any) => {
            console.error('Failed to check for local VOD: ', err)
        }).finally(() => {
            this.checkingForLocal = false
        })
///#endif
    }

    @Watch('track')
    refreshDownloadUri() {
        this.downloadUri = null
        if (!this.track){ 
            return
        }

        apiClient.accessToken().getVodSegment(this.track.segments[0].uri).then((resp : ApiData<string>) => {
            this.downloadUri = resp.data
        }).catch((err: any) => {
            console.error('Failed to get VOD download URI: ', err)
        })
    }

    doLocalDownload() {
///#if DESKTOP
        this.sendAnalyticsEvent(this.AnalyticsCategory.MatchVod, this.AnalyticsAction.DownloadVod, '', 0)
        manager.startDownload(this.videoUuid)
///#endif
    }

    openLocalDownload() {
        if (!this.localVodLocation) {
            return
        }
///#if DESKTOP
        shell.showItemInFolder(this.localVodLocation)
///#endif
    }

    onDownloadProgress(progress: number, finish: boolean, error: boolean) {
        if (finish) {
            this.downloadProgress = null
            this.downloadError = error
            this.recheckForLocalFile()
            this.$emit('on-download')
        } else {
            this.downloadProgress = progress
        }
    }

    @Watch('videoUuid')
    registerCallbacks() {
        this.unregisterCallbacks()

        this._registeredDownloadCb = this.onDownloadProgress.bind(this)
        manager.downloads.addCallback(this.videoUuid, this._registeredDownloadCb!)
    }

    unregisterCallbacks() {
        if (!!this._registeredDownloadCb) {
            manager.downloads.removeCallback(this.videoUuid, this._registeredDownloadCb!)
        }

        this._registeredDownloadCb = null
    }

    mounted () {
        this.recheckForLocalFile()
        this.refreshDownloadUri()
        this.registerCallbacks()
    }

    beforeDestroy() {
        this.unregisterCallbacks()
    }
}

</script>