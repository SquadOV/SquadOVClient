<template>
    <div class="d-flex align-center pa-2" v-if="options.length > 0">
        <div>
            <v-icon>
                mdi-eye
            </v-icon>
        </div>

        <v-divider vertical class="mx-2"></v-divider>

        <div class="d-flex flex-grow-1">
            <div
                v-for="(vod, idx) in options"
                :key="idx"
                @click="$emit('input', vod)"
                class="vod-select-div ml-1"
            >
                <slot
                    name="vod"
                    v-bind:ivod="vod"
                    v-bind:selected="value"
                ></slot>
            </div>
        </div>

        <div class="d-flex align-center" v-if="!!value && !this.value.isLocal && !hasFastify && !hasLocal">
            <div class="text-subtitle-2 font-weight-bold">
                VOD Processing...
            </div>

            <v-tooltip bottom :open-delay="100">
                <template v-slot:activator="{on, attrs}">
                    <div
                        v-on="on"
                        v-bind="attrs"
                    >
                        <v-progress-circular
                            indeterminate size="16"
                        >
                        </v-progress-circular>
                    </div>
                </template>

                This VOD is still being processed. Loading will be slow, the VOD may seem to be incomplete, and/or some features will be disabled until it is finished processing. Check back soon!
            </v-tooltip>
        </div>

        <template v-if="!!$store.state.currentUser">
            <v-divider vertical class="mx-2"></v-divider>
            <template v-if="!!value && value.userUuid === $store.state.currentUser.uuid && (hasFastify || hasLocal)">
                <!-- delete button -->
                <vod-delete-button
                    v-if="!!value"
                    :vod="value"
                    @on-delete="onDeleteFinish"
                >
                </vod-delete-button>

                <!-- upload button -->
                <v-tooltip bottom>
                    <template v-slot:activator="{on, attrs}">
                        <v-btn color="primary" icon v-if="canUpload" @click="uploadLocalVod" :loading="isUploading" v-on="on" v-bind="attrs">
                            <v-icon>
                                mdi-upload
                            </v-icon>
                        </v-btn>
                    </template>

                    Upload
                </v-tooltip>

                <div v-if="uploadProgress !== null">
                    {{ (uploadProgress * 100.0).toFixed(0) }}% 
                </div>
            </template>

            <!-- download VOD button -->
            <vod-download-button
                v-if="!!value"
                :video-uuid="value.videoUuid"
                :track="track"
                @on-download="recheckForLocalFile"
            >
            </vod-download-button>
            
            <!-- create clip button -->
            <v-tooltip bottom>
                <template v-slot:activator="{on, attrs}">
                    <div v-on="on" v-bind="attrs">
                        <v-btn :disabled="!isClippingEnabled" color="success" icon @click="openEditingWindow">
                            <v-icon>
                                mdi-content-cut
                            </v-icon>
                        </v-btn>
                    </div>
                </template>
                <span v-if="isClippingEnabled"> Clip </span>
                <span v-else> Go to Desktop Application to create a clip. </span>
            </v-tooltip>

            <!-- drawing button -->
            <v-tooltip bottom>
                <template v-slot:activator="{on, attrs}">
                    <v-btn color="pink accent-2" icon @click="toggleDrawing" :input-value="enableDraw" v-on="on" v-bind="attrs">
                        <v-icon>
                            mdi-palette
                        </v-icon>
                    </v-btn>
                </template>

                Draw
            </v-tooltip>

            <!-- clip library button -->
            <v-tooltip bottom>
                <template v-slot:activator="{on, attrs}">
                    <v-btn color="primary" icon @click="openClipWindowForMatch" v-if="hasFastify"  v-on="on" v-bind="attrs">
                        <v-icon>
                            mdi-filmstrip-box-multiple
                        </v-icon>
                    </v-btn>
                </template>

                Clip Library (Match)
            </v-tooltip>

            <!-- favorite -->
            <v-tooltip bottom>
                <template v-slot:activator="{on, attrs}">
                    <div v-on="on" v-bind="attrs">
                        <vod-favorite-button
                            v-if="!!value && !disableFavorite && hasFastify"
                            :vod-uuid="value.videoUuid"
                        >
                        </vod-favorite-button>
                    </div>
                </template>

                Favorite
            </v-tooltip>

            <!-- watch list -->
            <v-tooltip bottom>
                <template v-slot:activator="{on, attrs}">
                    <div v-on="on" v-bind="attrs">
                        <vod-watchlist-button
                            v-if="!!value && hasFastify"
                            :vod-uuid="value.videoUuid"
                        >
                        </vod-watchlist-button>
                    </div>
                </template>

                Add to Watch List
            </v-tooltip>

        </template>

        <v-snackbar
            v-model="downloadError"
            :timeout="5000"
            color="error"
        >
            Failed to download/delete the VOD, please try again later.
        </v-snackbar>

        <v-snackbar
            v-model="uploadError"
            :timeout="5000"
            color="error"
        >
            Failed to upload your locally recorded VOD, please submit a bug report.
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Component, {mixins} from 'vue-class-component'
import CommonComponent from '@client/vue/CommonComponent'
import { Prop, Watch } from 'vue-property-decorator'
import { VodAssociation } from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import { VodEditorContext, openVodEditingWindow } from '@client/js/vods/editor'
import * as vod from '@client/js/squadov/vod'
import * as pi from '@client/js/pages'
import { SquadOvGames } from '@client/js/squadov/game'
import { openPathInNewWindow } from '@client/js/external'
import VodFavoriteButton from '@client/vue/utility/vods/VodFavoriteButton.vue'
import VodWatchlistButton from '@client/vue/utility/vods/VodWatchlistButton.vue'
import { DownloadUploadProgressCb, manager } from '@client/js/vods/local_manager'
import { MatchVideoSharePermissions } from '@client/js/squadov/share'
import VodDownloadButton from '@client/vue/utility/vods/VodDownloadButton.vue'
import VodDeleteButton from '@client/vue/utility/vods/VodDeleteButton.vue'

/// #if DESKTOP
import { ipcRenderer } from 'electron'
import { IpcResponse } from '@client/js/system/ipc'
/// #endif

@Component({
    components: {
        VodFavoriteButton,
        VodWatchlistButton,
        VodDownloadButton,
        VodDeleteButton,
    }
})
export default class GenericVodPicker extends mixins(CommonComponent) {
    @Prop({required: true})
    value!: VodAssociation | null

    @Prop({required: true})
    options!: VodAssociation[]

    @Prop({required: true})
    matchUuid!: string

    @Prop()
    timestamp!: Date | undefined | null

    @Prop({required: true})
    game!: SquadOvGames

    @Prop({type: Boolean, default: false})
    disableFavorite!: boolean

    @Prop({type: Boolean, default: false})
    enableDraw!: boolean

    manifest: vod.VodManifest | null = null
    track: vod.VodTrack | null = null
    context: VodEditorContext | null = null
    localVodLocation: string | null = null

    permissions: MatchVideoSharePermissions | null = null

    @Watch('value')
    checkAnalytics(newVod: VodAssociation | null, oldVod: VodAssociation | null) {
        if (!!newVod && !!oldVod) {
            this.sendAnalyticsEvent(this.AnalyticsCategory.MatchVod, this.AnalyticsAction.ChangePov, '', this.options.length)
        }
    }

    @Watch('value')
    refreshPermissions() {
        this.permissions = null
        if (!this.value) {
            return
        }

        apiClient.accessToken().getGenericSharePermissions(undefined, this.value.videoUuid, undefined).then((resp: ApiData<MatchVideoSharePermissions>) => {
            this.permissions = resp.data
        }).catch((err: any) => {
            console.error('Failed to get video share permissions: ', err)
        })
    }

    get clippingAllowed(): boolean {
        return (this.hasFastify || this.hasLocal) && this.isClippingEnabled && !!this.permissions && this.permissions.canClip
    }

    @Watch('timestamp')
    onChangeTimestamp() {
        if (!this.context || !this.timestamp) {
            return
        }
        this.context.syncTime(this.timestamp)
    }

    get isClippingEnabled(): boolean {
///#if DESKTOP
        return true
///#else
        return false
///#endif
    }

    onDeleteFinish(videoUuid: string) {
        let newOptions = [...this.options]
        let idx = newOptions.findIndex((ele: VodAssociation) => ele.videoUuid === videoUuid)
        if (idx === -1) {
            return
        }

        newOptions.splice(idx, 1)
        this.$emit('update:options', newOptions)
        if (newOptions.length === 0) {
            this.$emit('input', null)
        } else {
            this.$emit('input', newOptions[0])
        }
    }

    @Watch('value')
    refreshManifest() {
        this.manifest = null
        if (!this.value) {
            return
        }
        apiClient.accessToken().getVodManifest(this.value.videoUuid!).then((resp : ApiData<vod.VodManifest>) => {
            this.manifest = resp.data
        }).catch((err: any) => {
            console.error('Failed to get VOD manifest:', err)
        })
    }

    @Watch('manifest')
    refreshTrack() {
        this.track = null
        if (!this.manifest) {
            return
        }

        let track = vod.getVideoQualityTrack(this.manifest, 'source')
        if (!track) {
            return
        }
        this.track = track
    }


    get useSimpleDownload(): boolean {
///#if DESKTOP
        return false
///#else
        return true
///#endif
    }

    downloadProgress: number | null = null
    downloadError: boolean = false
    checkingForLocal: boolean = false

    @Watch('value')
    recheckForLocalFile() {
        if (!this.value || this.checkingForLocal) {
            return
        }
///#if DESKTOP
        this.checkingForLocal = true
        ipcRenderer.invoke('check-vod-local', this.value.videoUuid).then((resp: IpcResponse<string>) => {
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
    
    get hasLocal(): boolean {
        return !!this.localVodLocation
    }

    get hasFastify(): boolean {
        if (!this.track) {
            return false
        }
        return this.track.segments[0].mimeType !== 'video/mp2t'
    }

    openClipWindowForMatch() {
        let path = this.$router.resolve({
            name: pi.ClipLibraryPageId,
            query: {
                matchUuid: this.matchUuid
            }
        })
        openPathInNewWindow(path.href)
    }

    openEditingWindow() {
        if (!this.value) {
            return
        }
        this.context = new VodEditorContext(this.value.videoUuid)
        this.context.startSource(this.value)
        this.onChangeTimestamp()

        this.sendAnalyticsEvent(this.AnalyticsCategory.MatchVod, this.AnalyticsAction.OpenCreateClip, '', 0)
        openVodEditingWindow(this.value.videoUuid, this.game)
    }

    toggleDrawing() {
        if (!this.enableDraw) {
            this.sendAnalyticsEvent(this.AnalyticsCategory.MatchVod, this.AnalyticsAction.StartDrawVod, '', 0)
        } else {
            this.sendAnalyticsEvent(this.AnalyticsCategory.MatchVod, this.AnalyticsAction.EndDrawVod, '', 0)
        }
        this.$emit('update:enableDraw', !this.enableDraw)
    }

    get canUpload(): boolean {
///#if DESKTOP
        if (!this.value || !this.localVodLocation || this.manualDisableUpload) {
            return false
        }
        return this.value.isLocal
///#else
        return false
///#endif
    }

    uploadProgress: number | null = null
    manualDisableUpload: boolean = false
    uploadError: boolean = false

    get isUploading(): boolean {
        return this.uploadProgress !== null
    }

    onUploadProgress(progress: number, finish: boolean, error: boolean) {
        if (finish) {
            this.uploadProgress = null
            this.uploadError = error
            this.manualDisableUpload = true
        } else {
            this.uploadProgress = progress
        }
    }

    uploadLocalVod() {
        if (!this.canUpload || !this.value || !this.localVodLocation) {
            return
        }
///#if DESKTOP
        this.sendAnalyticsEvent(this.AnalyticsCategory.MatchVod, this.AnalyticsAction.UploadVod, '', 0)
        manager.enqueueUpload(this.value.videoUuid)
///#endif
    }

    _registeredUploadCb: DownloadUploadProgressCb | null = null

    @Watch('value')
    registerCallbacks() {
        this.unregisterCallbacks()
        if (!this.value) {
            return
        }

        this._registeredUploadCb = this.onUploadProgress.bind(this)
        manager.uploads.addCallback(this.value!.videoUuid, this._registeredUploadCb!)
    }

    unregisterCallbacks() {
        if (!!this.value && !!this._registeredUploadCb) {
            manager.uploads.removeCallback(this.value!.videoUuid, this._registeredUploadCb!)
        }

        this._registeredUploadCb = null
    }

    mounted () {
        this.refreshManifest()
        this.recheckForLocalFile()
        this.registerCallbacks()
        this.refreshPermissions()
    }

    beforeDestroy() {
        this.unregisterCallbacks()
    }
}

</script>

<style scoped>

.vod-select-div {
    cursor: pointer;
}

</style>