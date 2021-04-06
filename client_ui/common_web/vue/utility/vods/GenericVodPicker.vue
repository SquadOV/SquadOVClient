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
                class="vod-select-div"
            >
                <slot
                    name="vod"
                    v-bind:ivod="vod"
                    v-bind:selected="value"
                ></slot>
            </div>
        </div>

        <div class="d-flex align-center" v-if="!hasFastify && !hasLocal">
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
                <!-- delete VOD button -->
                <v-dialog v-model="showHideDeleteConfirm" persistent max-width="40%">
                    <template v-slot:activator="{on, attrs}">
                        <v-btn
                            color="error"
                            icon
                            v-on="on"
                            v-bind="attrs"
                            :loading="loadingDelete || loadingLocalDelete"
                        >
                            <v-icon>
                                mdi-delete
                            </v-icon>
                        </v-btn>
                    </template>

                    <v-card>
                        <v-card-title>
                            Confirmation
                        </v-card-title>
                        <v-divider></v-divider>

                        <v-card-text class="mt-4">
                            <div>
                                Are you sure you wish to delete your VOD? You and your squadmates will no longer be able to watch this match from your point of view.
                                This will delete the VOD from SquadOV's servers as well as from your local machine (if applicable).
                            </div>

                            <div class="mt-4">
                                Please type <span class="font-weight-bold">CONFIRM</span> (case-sensitive) to confirm this action.
                            </div>

                            <v-text-field
                                class="mt-4"
                                filled
                                label="Confirmation"
                                v-model="confirmationText"
                                hide-details
                            >
                            </v-text-field>
                        </v-card-text>

                        <v-card-actions>
                            <v-btn @click="hideDeleteConfirm">
                                Cancel
                            </v-btn>

                            <v-spacer></v-spacer>

                            <v-btn
                                color="error"
                                :loading="loadingDelete"
                                :disabled="confirmationText != 'CONFIRM'"
                                @click="deleteVod"
                            >
                                Delete
                            </v-btn>
                        </v-card-actions>
                    </v-card>
                </v-dialog>

                <!-- download VOD button -->
                <v-btn color="warning icon" icon v-if="hasLocal" @click="openLocalDownload" :loading="checkingForLocal">
                    <v-icon>
                        mdi-folder-open
                    </v-icon>
                </v-btn>

                <v-btn color="warning" icon v-else-if="!!downloadUri && useSimpleDownload && hasFastify" :href="downloadUri" :loading="checkingForLocal">
                    <v-icon>
                        mdi-download
                    </v-icon>
                </v-btn>

                <v-btn color="warning" icon v-else-if="!useSimpleDownload && hasFastify" @click="doLocalDownload" :loading="downloadProgress !== null || checkingForLocal">
                    <v-icon>
                        mdi-download
                    </v-icon>
                </v-btn>

                <div v-if="downloadProgress !== null">
                    {{ (downloadProgress * 100.0).toFixed(0) }}% 
                </div>
                
                <!-- create clip button -->
                <v-btn color="success" icon v-if="hasFastify && isClippingEnabled" @click="openEditingWindow">
                    <v-icon>
                        mdi-content-cut
                    </v-icon>
                </v-btn>
            </template>

            <!-- drawing button -->
            <v-btn color="pink accent-2" icon @click="toggleDrawing" :input-value="enableDraw">
                <v-icon>
                    mdi-palette
                </v-icon>
            </v-btn>

            <!-- clip library button -->
            <v-btn color="primary" icon @click="openClipWindowForMatch" v-if="hasFastify">
                <v-icon>
                    mdi-filmstrip-box-multiple
                </v-icon>
            </v-btn>

            <!-- favorite -->
            <vod-favorite-button
                v-if="!!value && !disableFavorite && hasFastify"
                :vod-uuid="value.videoUuid"
            >
            </vod-favorite-button>

            <!-- watch list -->
            <vod-watchlist-button
                v-if="!!value && hasFastify"
                :vod-uuid="value.videoUuid"
            >
            </vod-watchlist-button>

        </template>

        <v-snackbar
            v-model="downloadError"
            :timeout="5000"
            color="error"
        >
            Failed to download/delete the VOD, please try again later.
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { VodAssociation } from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import { VodEditorContext, openVodEditingWindow } from '@client/js/vods/editor'
import * as vod from '@client/js/squadov/vod'
import * as pi from '@client/js/pages'
import { SquadOvGames } from '@client/js/squadov/game'
import { openPathInNewWindow } from '@client/js/external'
import { DownloadProgress } from '@client/js/system/download'
import VodFavoriteButton from '@client/vue/utility/vods/VodFavoriteButton.vue'
import VodWatchlistButton from '@client/vue/utility/vods/VodWatchlistButton.vue'
import { VodRemoteControlContext } from '@client/js/vods/remote'

/// #if DESKTOP
import { shell, ipcRenderer } from 'electron'
import { IpcResponse } from '@client/js/system/ipc'
/// #endif

@Component({
    components: {
        VodFavoriteButton,
        VodWatchlistButton,
    }
})
export default class GenericVodPicker extends Vue {
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

    showHideDeleteConfirm: boolean = false
    loadingDelete: boolean = false
    loadingLocalDelete: boolean = false
    confirmationText: string = ''

    manifest: vod.VodManifest | null = null
    track: vod.VodTrack | null = null
    downloadUri: string | null = null
    context: VodEditorContext | null = null
    localVodLocation: string | null = null
    rcContext: VodRemoteControlContext | null = null

    @Watch('timestamp')
    onChangeTimestamp() {
        if (!this.context || !this.timestamp) {
            return
        }
        this.context.syncTime(this.timestamp)
    }

    @Watch('value')
    refreshRcContext() {
        if (!this.value) {
            return
        }

        this.rcContext = new VodRemoteControlContext(this.value.videoUuid)
    }

    get isClippingEnabled(): boolean {
///#if DESKTOP
        return true
///#else
        return false
///#endif
    }

    hideDeleteConfirm() {
        this.showHideDeleteConfirm = false
        this.confirmationText = ''
    }

    deleteVod() {
        if (!this.value) {
            return
        }

        this.loadingDelete = true
        this.loadingLocalDelete = this.hasLocal

        apiClient.deleteVod(this.value.videoUuid).then(() => {
            if (!this.loadingLocalDelete) {
                this.hideDeleteConfirm()
                this.$router.go(0)
            }
        }).catch((err: any) => {
            console.log('Failed to delete VOD: ', err)
            this.downloadError = true
        }).finally(() => {
            this.loadingDelete = false
        })

///#if DESKTOP
        if (this.hasLocal && !!this.rcContext) {
            // Need a delay to give us time to release the file handle on the client side by destorying the player.
            this.rcContext.stopAndDestroy()
            setTimeout(() => {
                ipcRenderer.invoke('delete-vod-local', this.value!.videoUuid).then((resp: IpcResponse<void>) => {
                    if (resp.success) {
                        if (!this.loadingDelete) {
                            this.hideDeleteConfirm()
                            this.$router.go(0)
                        }
                    } else {
                        this.downloadError = true
                    }
                }).catch((err: any) => {
                    console.log('Failed to delete local VOD: ', err)
                    this.downloadError = true
                }).finally(() => {
                    this.loadingLocalDelete = false
                })
            }, 1000)
        }
///#endif
    }

    @Watch('value')
    refreshManifest() {
        this.manifest = null
        if (!this.value) {
            return
        }
        apiClient.getVodManifest(this.value.videoUuid!).then((resp : ApiData<vod.VodManifest>) => {
            this.manifest = resp.data
        }).catch((err: any) => {
            console.log('Failed to get VOD manifest:', err)
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
            console.log('Failed to check for local VOD: ', err)
        }).finally(() => {
            this.checkingForLocal = false
        })
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

    doLocalDownload() {
        if (!this.value) {
            return
        }
///#if DESKTOP
        this.downloadProgress = 0

        let progressNotif = (e: any, info: DownloadProgress) => {
            if (info.done) {
                ipcRenderer.removeListener('vod-download-progress', progressNotif)
                this.downloadProgress = null
                this.downloadError = (info.success === false)
                this.recheckForLocalFile()
            } else if (info.download !== undefined && info.total !== undefined) {
                if (info.total === 0) {
                    this.downloadProgress = 0
                } else {
                    this.downloadProgress = info.download / info.total
                }
            }
        }

        ipcRenderer.send('request-vod-local-download', this.value.videoUuid)
        ipcRenderer.on('vod-download-progress', progressNotif)
///#endif
    }

    @Watch('track')
    refreshDownloadUri() {
        this.downloadUri = null
        if (!this.track){ 
            return
        }

        apiClient.getVodSegment(this.track.segments[0].uri).then((resp : ApiData<string>) => {
            this.downloadUri = resp.data
        }).catch((err: any) => {
            console.log('Failed to get VOD download URI: ', err)
        })
    }

    get downloadName(): string {
        if (!this.track) {
            return ''
        }
        return (this.track!.segments[0].mimeType === 'video/mp2t') ? 'vod.ts' : 'vod.mp4'
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
        openVodEditingWindow(this.value.videoUuid, this.game)
    }

    toggleDrawing() {
        this.$emit('update:enableDraw', !this.enableDraw)
    }

    mounted () {
        this.refreshManifest()
        this.recheckForLocalFile()
        this.refreshRcContext()
    }
}

</script>

<style scoped>

.vod-select-div {
    cursor: pointer;
}

</style>