<template>
    <div class="d-flex flex-column justify-space-between full-width full-parent-height" v-if="enabled && !!vod">
        <video-player
            class="mb-1"
            id="editor-vod"
            :vod="vod"
            :current-time="timestamp"
            @update:currentTime="updateTimestampFromPlayer"
            disable-theater
            fill
            ref="player"
            :loop-clip="previewClip"
            :clip-start="previewClip ? clipStart : undefined"
            :clip-end="previewClip ? clipEnd : undefined"
        >
        </video-player>

        <div class="mb-1 mx-2">
            <div id="editor-timeline">
                <generic-match-timeline
                    class="px-4"
                    :x-padding="16"
                    :start="start"
                    :end="end"
                    :current="currentTimestamp"
                    :input-events="events"
                    :major-tick-every="60"
                    :clip-start-handle="clipStart"
                    @update:clipStartHandle="setClipStart"
                    :clip-end-handle="clipEnd"
                    @update:clipEndHandle="setClipEnd"
                    @go-to-timestamp="updateTimestampFromTimeline"
                    fill
                >
                    <template v-slot:tick="{ tick }">
                        {{ secondsToTimeString(tick) }}
                    </template>
                </generic-match-timeline>
            </div>
        </div>

        <div class="d-flex align-center mb-1 mx-2">
            <v-checkbox
                class="mx-1"
                v-model="syncTimestamp"
                label="Sync Timestamp"
                hide-details
                dense
            >
            </v-checkbox>

            <v-checkbox
                class="mx-1"
                v-model="enableAudio"
                label="Enable Audio"
                hide-details
                dense
            >
            </v-checkbox>

            <v-checkbox
                class="mx-1"
                v-model="previewClip"
                label="Preview Clip"
                hide-details
                dense
            >
            </v-checkbox>

            <v-spacer></v-spacer>

            <v-text-field
                class="flex-grow-0"
                label="Start"
                :value="clipStartStr"
                @change="changeClipStart"
                hide-details
                dense
                solo
                single-line
                :key="`start-${startKey}`"
            >
            </v-text-field>

            <div class="mx-1">
                to
            </div>

            <v-text-field
                class="flex-grow-0"
                label="End"
                :value="clipEndStr"
                @change="changeClipEnd"
                hide-details
                dense
                solo
                single-line
                :key="`end-${endKey}`"
            >
            </v-text-field>
        </div>

        <div class="mb-1 mx-2">
            <v-btn
                color="success"
                block
                @click="doClip"
            >
                Clip it!
            </v-btn>
        </div>

        <v-dialog
            :value="showHideClipDialog"
            persistent
            max-width="60%"
        >
            <v-card :key="clipKey">
                <v-card-title>
                    Your New Clip
                </v-card-title>
                <v-divider></v-divider>
            
                <template v-if="clipInProgress">
                    <div id="clip-progress-div" class="d-flex flex-column justify-center align-center">
                        <div class="text-h6 font-weight-bold mb-2">
                            Clipping in progress! One moment please...
                        </div>
                        <v-progress-circular indeterminate size="64"></v-progress-circular>
                    </div>
                </template>

                <template v-else>
                    <div class="ma-2">
                        <div id="editor-clip-preview">
                            <video class="video-js vjs-fill" ref="video">
                            </video>
                        </div>
                    </div>

                    <v-card-actions>
                        <v-btn
                            color="error"
                            @click="cancelClip"
                        >
                            Cancel
                        </v-btn>

                        <v-spacer></v-spacer>

                        <v-btn
                            color="success"
                        >
                            Save
                        </v-btn>
                    </v-card-actions>
                </template>
            </v-card>
        </v-dialog>

        <v-snackbar
            v-model="clipError"
            :timeout="5000"
            color="error"
        >
            Failed to create the clip. Please try again (and submit a bug report!).
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { VodEditorContext, requestVodClip } from '@client/js/vods/editor'
import { VodAssociation } from '@client/js/squadov/vod'
import { GenericEvent} from '@client/js/event'
import { secondsToTimeString, timeStringToSeconds } from '@client/js/time'

import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'
import GenericMatchTimeline from '@client/vue/utility/GenericMatchTimeline.vue'
import videojs from 'video.js'
import 'video.js/dist/video-js.css' 
import fs from 'fs'

const MAX_CLIP_LENGTH_SECONDS = 45

@Component({
    components: {
        VideoPlayer,
        GenericMatchTimeline
    }
})
export default class VodEditor extends Vue {
    secondsToTimeString = secondsToTimeString

    @Prop({required: true})
    videoUuid!: string

    context: VodEditorContext | undefined = undefined
    vod: VodAssociation | null = null

    currentTimestamp: number = 0
    clipStart: number = 0
    clipEnd: number = 0

    syncTimestamp: boolean = true
    enableAudio: boolean = true
    previewClip: boolean = false

    startKey: number = 0
    endKey: number = 0

    clipInProgress: boolean = false
    showHideClipDialog: boolean = false
    clipError: boolean = false
    localClipPath: string | null = null
    clipKey: number = 0

    $refs!: {
        player: VideoPlayer
        video: HTMLVideoElement
    }
    player: videojs.Player | null = null

    enabled(): boolean {
///#if DESKTOP
        return true
///#else
        return false
///#endif
    }

    updateTimestampFromTimeline(t: number) {
        if (!this.vod) {
            return
        }

        let dt = new Date(this.vod.startTime.getTime() + t * 1000.0)
        this.updateTimestampFromContext(dt, true)
    }

    updateTimestampFromPlayer(dt: Date) {
        if (!this.vod) {
            return
        }

        this.currentTimestamp = (dt.getTime() - this.vod.startTime.getTime()) / 1000.0
    }

    updateTimestampFromContext(dt: Date, force: boolean = false) {
        if (!this.syncTimestamp && !force) {
            return
        }

        this.updateTimestampFromPlayer(dt)

        if (!!this.vod) {
            this.$refs.player.goToTimeMs(dt.getTime() - this.vod.startTime.getTime())
        }
    }

    get timestamp(): Date {
        if (!this.vod) {
            return new Date()
        }
        return new Date(this.vod.startTime.getTime() + this.currentTimestamp * 1000.0)
    }

    resetClip() {
        this.clipStart = 0
        this.clipEnd = Math.min(this.clipStart + MAX_CLIP_LENGTH_SECONDS, this.end)
    }

    @Watch('videoUuid')
    refreshContext() {
        if (!this.enabled) {
            return
        }
        this.context = new VodEditorContext(this.videoUuid)
        this.context.addTimeSyncListener((e: Date) => {
            this.updateTimestampFromContext(e)
        })
        this.context.requestVodAssociation().then((v: VodAssociation) => {
            this.vod = v
            this.resetClip()
        }).catch((err: any) => {
            console.log('Failed to request VOD association: ', err)
        })
    }
    
    mounted() {
        this.refreshContext()
    }

    beforeDestroy() {
        if (!!this.context) {
            this.context.close()
        }
        this.context = undefined
        this.cancelClip()
    }

    get start(): number {
        return 0
    }

    get end(): number {
        if (!this.vod) {
            return 0
        }
        return (this.vod.endTime.getTime() - this.vod.startTime.getTime()) / 1000.0
    }

    get events(): GenericEvent[] {
        return []
    }

    get clipStartStr(): string {
        return secondsToTimeString(this.clipStart)
    }

    get clipDuration(): number {
        return this.clipEnd - this.clipStart
    }
    
    changeClipStart(s: string) {
        this.setClipStart(timeStringToSeconds(s))
    }

    setClipStart(s: number) {
        this.clipStart = Math.min(Math.max(s, this.start), this.clipEnd)
        if (this.clipDuration > MAX_CLIP_LENGTH_SECONDS) {
            this.clipEnd = this.clipStart + MAX_CLIP_LENGTH_SECONDS
        }
        this.startKey += 1
    }

    get clipEndStr(): string {
        return secondsToTimeString(this.clipEnd)
    }

    changeClipEnd(s: string) {
        this.setClipEnd(timeStringToSeconds(s))
    }

    setClipEnd(s: number) {
        this.clipEnd = Math.min(Math.max(s, this.clipStart), this.end)
        if (this.clipDuration > MAX_CLIP_LENGTH_SECONDS) {
            this.clipStart = this.clipEnd - MAX_CLIP_LENGTH_SECONDS
        }
        this.endKey += 1
    }

    cancelClip() {
        if (!this.localClipPath) {
            return
        }
        this.showHideClipDialog = false
        this.clipInProgress = false
        if (!!this.player) {
            this.player.dispose()
        }
        this.player = null

        if (fs.existsSync(this.localClipPath)) {
            fs.unlinkSync(this.localClipPath)
        }
    }

    doClip() {
        if (this.clipInProgress) {
            return
        }

        let videoUri = this.$refs.player.videoUri
        if (!videoUri) {
            return
        }

        this.clipInProgress = true
        this.showHideClipDialog = true
        requestVodClip(videoUri, this.clipStart, this.clipEnd).then((resp: string) => {
            let normalPath = resp.replace(/\\/g, '/')
            this.clipInProgress = false
            this.clipKey += 1
            this.localClipPath = `file:///${normalPath}`
        }).catch((err: any) => {
            console.log('Failed to clip: ', err)
            this.clipError = true
            this.showHideClipDialog = false
            this.clipInProgress = false
        })
    }

    @Watch('localClipPath')
    refreshVideoPlayer() {
        if (!this.localClipPath) {
            return
        }

        Vue.nextTick(() => {
            this.player = videojs(this.$refs.video, {
                controls: true,
                autoplay: false,
                preload: 'auto',
                playbackRates: [
                    0.25,
                    0.5,
                    1.0,
                    1.25,
                    1.5,
                    2.0
                ]
            })

            this.player.src([
                {
                    src: this.localClipPath!,
                    type: 'video/mp4',
                },
            ])
        })
    }
}

</script>

<style scoped>

#editor-vod {
    height: 75vh;
    max-height: 75vh;
}

#editor-timeline {
    height: 10vh;
    max-height: 10vh;
}

#clip-progress-div {
    height: 500px;
}

#editor-clip-preview {
    width: 100%;
    height: 400px;
}

>>>.v-input--selection-controls {
    margin-top: 0px !important;
    padding-top: 0px !important;
}

</style>