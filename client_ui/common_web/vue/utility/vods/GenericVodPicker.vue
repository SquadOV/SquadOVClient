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
                @click="$emit('input', arguments[0])"
                class="vod-select-div"
            >
                <slot
                    name="vod"
                    v-bind:ivod="vod"
                    v-bind:selected="value"
                ></slot>
            </div>
        </div>

        <v-tooltip bottom :open-delay="1000">
            <template v-slot:activator="{on, attrs}">
                <div
                    v-if="!hasFastify"
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

        <template v-if="!!$store.state.currentUser">
            <v-divider vertical class="mx-2"></v-divider>

            <template v-if="!!value && value.userUuid === $store.state.currentUser.uuid && hasFastify">
                <!-- delete VOD button -->
                <v-dialog v-model="showHideDeleteConfirm" persistent max-width="40%">
                    <template v-slot:activator="{on, attrs}">
                        <v-btn
                            color="error"
                            icon
                            v-on="on"
                            v-bind="attrs"
                            :loading="loadingDelete"
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
                <v-btn color="warning" icon v-if="!!downloadUri" :href="downloadUri">
                    <v-icon>
                        mdi-download
                    </v-icon>
                </v-btn>
                
                <!-- create clip button -->
                <v-btn color="success" icon v-if="hasFastify && isClippingEnabled" @click="openEditingWindow">
                    <v-icon>
                        mdi-content-cut
                    </v-icon>
                </v-btn>
            </template>

            <!-- clip library button -->
            <v-btn color="primary" icon @click="openClipWindowForMatch">
                <v-icon>
                    mdi-filmstrip-box-multiple
                </v-icon>
            </v-btn>
        </template>
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

@Component
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

    showHideDeleteConfirm: boolean = false
    loadingDelete: boolean = false
    confirmationText: string = ''

    manifest: vod.VodManifest | null = null
    track: vod.VodTrack | null = null
    downloadUri: string | null = null
    context: VodEditorContext | null = null

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

    hideDeleteConfirm() {
        this.showHideDeleteConfirm = false
        this.confirmationText = ''
    }

    deleteVod() {
        if (!this.value) {
            return
        }

        this.loadingDelete = true
        apiClient.deleteVod(this.value.videoUuid).then(() => {
            this.hideDeleteConfirm()
            this.$router.go(0)
        }).catch((err: any) => {
            console.log('Failed to delete VOD: ', err)
        }).finally(() => {
            this.loadingDelete = false
        })
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
        openVodEditingWindow(this.value.videoUuid, this.game)
    }


    mounted () {
        this.refreshManifest()
    }
}

</script>

<style scoped>

.vod-select-div {
    cursor: pointer;
}

</style>