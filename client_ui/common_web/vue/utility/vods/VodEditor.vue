<template>
    <div class="d-flex flex-column justify-space-between full-width full-parent-height" v-if="enabled && !!vod">
        <video-player
            class="mb-1"
            id="editor-vod"
            :vod="vod"
            :current-time="timestamp"
            @update:currentTime="updateTimestampFromPlayer"
            disable-theater
            disable-popout
            fill
            ref="player"
            :loop-clip="previewClip"
            :clip-start="previewClip ? clipStart / 1000.0 : undefined"
            :clip-end="previewClip ? clipEnd / 1000.0 : undefined"
            :disable-shortcuts="showHideClipDialog"
        >
        </video-player>

        <div class="mb-1 mx-2">
            <div class="px-2 editor-timeline">
                <generic-match-timeline
                    :start="start"
                    :end="end"
                    :current="currentTimestamp"
                    :input-events="events"
                    :major-tick-every="majorTicksEvery"
                    :interval-ticks="minorIntervalTicks"
                    @go-to-timestamp="updateTimestampFromTimeline"
                    fill
                >
                    <template v-slot:tick="{ tick }">
                        {{ secondsToTimeString(tick / 1000.0) }}
                    </template>
                </generic-match-timeline>
            </div>

            <div class="px-2 editor-timeline mt-1">
                <generic-match-timeline
                    :start="start"
                    :end="end"
                    :current="currentTimestamp"
                    :input-events="events"
                    :major-tick-every="majorTicksEvery"
                    :interval-ticks="minorIntervalTicks"
                    :clip-start-handle="clipStart"
                    @update:clipStartHandle="setClipStart"
                    :clip-end-handle="clipEnd"
                    @update:clipEndHandle="setClipEnd"
                    fill
                    disable-seeking
                >
                    <template v-slot:tick="{ tick }">
                        {{ secondsToTimeString(tick / 1000.0) }}
                    </template>
                </generic-match-timeline>
            </div>

            <div class="mt-2">
                <pricing-notifier-wrapper
                    :tier="EPricingTier.Gold"
                    shrink
                >
                    Max Clip Length (seconds): {{ maxClipLengthMilliseconds / 1000 }}
                </pricing-notifier-wrapper>
            </div>
        </div>

        <div class="d-flex align-center mb-1 mx-2">
            <v-checkbox
                class="mx-1"
                v-model="useServerSideClipping"
                label="Server-Side Clipping"
                hide-details
                dense
                :disabled="!canDoServerSideClipping"
                v-if="canDoServerSideClipping"
            >
            </v-checkbox>

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
                v-if="false"
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

            <v-text-field
                class="flex-grow-0"
                prepend-icon="mdi-timer"
                :value="currentTimeStr"
                hide-details
                dense
                solo
                single-line
                readonly
            >
            </v-text-field>

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
                <template v-slot:append>
                    <v-btn icon @click="setClipStart(currentTimestamp)" class="import-button">
                        <v-icon>
                            mdi-import
                        </v-icon>
                    </v-btn>
                </template>
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
                <template v-slot:append>
                    <v-btn icon @click="setClipEnd(currentTimestamp)" class="import-button">
                        <v-icon>
                            mdi-import
                        </v-icon>
                    </v-btn>
                </template>
            </v-text-field>
        </div>

        <div class="mb-1 mx-2">
            <v-btn
                color="success"
                block
                @click="doClip"
                :disabled="clipDuration == 0"
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
            
                <div id="clip-progress-div" class="d-flex flex-column justify-center align-center" :style="progressStyle">
                    <div class="text-h6 font-weight-bold mb-2">
                        Clipping in progress! One moment please...
                    </div>
                    <v-progress-circular indeterminate size="64"></v-progress-circular>
                </div>

                <template v-if="!clipInProgress && !clipUuid">
                    <div class="ma-2">
                        <video-player
                            v-if="!!localClipPath"
                            id="editor-clip-preview"
                            :vod="undefined"
                            fill
                            loop-clip
                            :override-uri="localClipPath"
                            disable-shortcuts
                        >
                        </video-player>

                        <v-form v-model="formValid">
                            <v-text-field
                                v-model="clipTitle"
                                label="Title"
                                filled
                                :rules="titleRules"
                                :readonly="!!clipUuid"
                            >
                            </v-text-field>

                            <v-textarea
                                filled
                                label="Description"
                                v-model="clipDescription"
                                hide-details
                                :readonly="!!clipUuid"
                            >
                            </v-textarea>
                        </v-form>
                    </div>

                    <v-card-actions v-if="!clipUuid">
                        <v-btn
                            color="error"
                            @click="cancelClip"
                        >
                            Cancel
                        </v-btn>

                        <v-spacer></v-spacer>

                        <v-btn
                            color="success"
                            :disabled="!formValid"
                            :loading="saveInProgress"
                            @click="saveClip"
                        >
                            Save
                        </v-btn>
                    </v-card-actions>
                </template>

                <template v-else-if="!!clipUuid">
                    <v-container fluid>
                        <v-row justify="center" align="center">
                            <v-col cols="6">
                                <v-btn class="mb-4" color="primary" block large @click="goToClip">
                                    Go to Clip
                                </v-btn>

                                <v-btn class="mb-4" color="success" block large @click="createNewClip">
                                    Create a new clip!
                                </v-btn>

                                <v-btn block large @click="returnToMatch">
                                    Return to Match
                                </v-btn>
                            </v-col>
                        </v-row>
                    </v-container>
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

        <v-snackbar
            v-model="badClipState"
            :timeout="5000"
            color="error"
        >
            Invalid clip parameters. Please try again (and submit a bug report!).
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Component, {mixins} from 'vue-class-component'
import CommonComponent from '@client/vue/CommonComponent'
import { Prop, Watch } from 'vue-property-decorator'
import { VodEditorContext, requestVodClip } from '@client/js/vods/editor'
import { VodAssociation, VodMetadata } from '@client/js/squadov/vod'
import { GenericEvent} from '@client/js/event'
import { secondsToTimeString, millisecondsToTimeString, timeStringToMilliseconds } from '@client/js/time'

import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'
import GenericMatchTimeline from '@client/vue/utility/GenericMatchTimeline.vue'

///#if DESKTOP
import fs from 'fs'
import { ipcRenderer } from 'electron'
///#endif
import { apiClient, ApiData, StagedClipStatusResponse } from '@client/js/api'
import { SquadOvGames } from '@client/js/squadov/game'
import * as pi from '@client/js/pages'
import PricingNotifierWrapper from '@client/vue/utility/squadov/PricingNotifierWrapper.vue'
import { EPricingTier } from '@client/js/squadov/pricing'

@Component({
    components: {
        VideoPlayer,
        GenericMatchTimeline,
        PricingNotifierWrapper,
    }
})
export default class VodEditor extends mixins(CommonComponent) {
    secondsToTimeString = secondsToTimeString
    EPricingTier = EPricingTier

    @Prop({required: true})
    videoUuid!: string

    @Prop({type: Number, required: true})
    game!: SquadOvGames

    context: VodEditorContext | undefined = undefined
    vod: VodAssociation | null = null

    currentTimestamp: number = 0
    clipStart: number = 0
    clipEnd: number = 0

    syncTimestamp: boolean = true
    enableAudio: boolean = true
    previewClip: boolean = false
    initialSync: boolean = false

    startKey: number = 0
    endKey: number = 0

    clipInProgress: boolean = false
    showHideClipDialog: boolean = false
    clipError: boolean = false
    localClipPath: string | null = null
    metadata: VodMetadata | null = null

    clipKey: number = 0
    numberOfTicks: number = 25.0
    expectedMaxScreenResolution: number = 1920
    intervalTicks: number = 5

    formValid: boolean = false
    clipTitle: string = ''
    clipDescription: string = ''
    clipPathsInSession: string[] = []
    saveInProgress: boolean = false

    clipUuid: string | null = null
    stagedClipUuid: string | null = null
    badClipState: boolean = false
    useServerSideClipping: boolean = true

    $refs!: {
        player: VideoPlayer
        urlInput: any
    }

    get maxClipLengthMilliseconds(): number {
        return this.$store.state.features.maxClipSeconds * 1000
    }

    get canDoServerSideClipping(): boolean {
        return !!this.vod && !this.vod.isLocal
    }

    get videoDurationMs(): number {
        return this.end - this.start
    }

    get minorIntervalTicks(): number {
        this.intervalTicks = this.calculateMinorTicks()
        return this.intervalTicks
    }
    
    get majorTicksEvery(): number {
        this.numberOfTicks = this.calculateMajorTicks()
        let ticks = this.videoDurationMs / this.numberOfTicks
        return Math.round(ticks)
    }

    calculateMajorTicks(): number {
        if (window.innerWidth >= this.expectedMaxScreenResolution * .95) {
            return 25.0
        } else if (window.innerWidth >= this.expectedMaxScreenResolution * .8) {
            return 20.0
        } else if (window.innerWidth >= this.expectedMaxScreenResolution * .6) {
            return 15.0
        }
        return 10.0
    }

    calculateMinorTicks(): number {
        if(window.innerWidth >= this.expectedMaxScreenResolution * .8) {
            return 10
        }
        return 5
    }

    enabled(): boolean {
///#if DESKTOP
        return true
///#else
        return false
///#endif
    }

    get progressStyle(): any {
        if (!!this.clipInProgress) {
            return {}
        } else {
            return {
                'display': 'none !important'
            }
        }
    }

    get titleRules() : any[] {
        return [
            (value : any) => (!!value && value.length > 0) || 'Required.',
        ]
    }

    updateTimestampFromTimeline(t: number) {
        if (!this.vod) {
            return
        }

        let dt = new Date(this.vod.startTime.getTime() + t)
        this.updateTimestampFromContext(dt, true)
    }

    updateTimestampFromPlayer(dt: Date) {
        if (!this.vod) {
            return
        }

        this.currentTimestamp = dt.getTime() - this.vod.startTime.getTime()
    }

    updateTimestampFromContext(dt: Date, force: boolean = false) {
        if (this.initialSync && (!this.syncTimestamp && !force)) {
            return
        }

        this.updateTimestampFromPlayer(dt)

        if (!this.initialSync) {
            this.setClipStart(this.currentTimestamp)
            this.setClipEnd(this.currentTimestamp + this.maxClipLengthMilliseconds)
            this.$refs.player.setPinned(dt)
            this.initialSync = true
        }

        if (!!this.vod) {
            this.$refs.player.goToTimeMs(dt.getTime() - this.vod.startTime.getTime(), false, false)
        }
    }

    get timestamp(): Date {
        if (!this.vod) {
            return new Date()
        }
        return new Date(this.vod.startTime.getTime() + this.currentTimestamp)
    }

    adjustTickers() {
        this.numberOfTicks = this.calculateMajorTicks()
        this.intervalTicks = this.calculateMinorTicks()
    }

    resetClip() {
        this.clipStart = 0
        this.clipEnd = Math.min(this.clipStart + this.maxClipLengthMilliseconds, this.end)
    }

    @Watch('videoUuid')
    refreshContext() {
        if (!this.enabled) {
            return
        }
        this.context = new VodEditorContext(this.videoUuid)
        this.context.requestVodAssociation().then((v: VodAssociation) => {
            this.vod = v
            this.resetClip()
            this.context!.requestTimeSync()
        }).catch((err: any) => {
            console.error('Failed to request VOD association: ', err)
        })
    }
    
    mounted() {
        window.addEventListener('resize', this.adjustTickers);
        this.refreshContext()
    }

    beforeDestroy() {
        if (!!this.context) {
            this.context.close()
        }
        this.context = undefined
        this.cancelClip()
        window.removeEventListener('resize', this.adjustTickers);
    }

    get start(): number {
        return 0
    }

    get end(): number {
        if (!this.vod) {
            return 0
        }
        return this.vod.endTime.getTime() - this.vod.startTime.getTime()
    }

    get events(): GenericEvent[] {
        return []
    }

    get currentTimeStr(): string {
        return millisecondsToTimeString(this.currentTimestamp)
    }

    get clipStartStr(): string {
        return millisecondsToTimeString(this.clipStart)
    }

    get clipDuration(): number {
        return this.clipEnd - this.clipStart
    }
    
    changeClipStart(s: string) {
        this.setClipStart(timeStringToMilliseconds(s))
    }

    setClipStart(s: number) {
        let oldDuration = this.clipDuration
        this.clipStart = Math.min(Math.max(s, this.start), this.end)
        if (this.clipDuration < 0) {
            this.clipEnd = Math.min(this.clipStart + oldDuration, this.end)
        } else if (this.clipDuration > this.maxClipLengthMilliseconds) {
            this.clipEnd = Math.min(this.clipStart + this.maxClipLengthMilliseconds, this.end)
        }
        this.startKey += 1
    }

    get clipEndStr(): string {
        return millisecondsToTimeString(this.clipEnd)
    }

    changeClipEnd(s: string) {
        this.setClipEnd(timeStringToMilliseconds(s))
    }

    setClipEnd(s: number) {
        let oldDuration = this.clipDuration
        this.clipEnd = Math.min(Math.max(s, this.start), this.end)
        if (this.clipDuration < 0) {
            this.clipStart = Math.max(this.clipEnd - oldDuration, this.start)
        } else if (this.clipDuration > this.maxClipLengthMilliseconds) {
            this.clipStart = Math.max(this.clipEnd - this.maxClipLengthMilliseconds, this.start)
        }
        this.endKey += 1
    }

    cancelClip() {
        // We also need to make sure the clip gets deleted in local storage.
///#if DESKTOP
        for (let path of this.clipPathsInSession) {
            if (fs.existsSync(path)) {
                fs.unlinkSync(path)
            }
        }
///#endif
        this.clipPathsInSession = []

        this.showHideClipDialog = false
        this.clipInProgress = false

        this.clipTitle = ''
        this.clipDescription = ''
        this.localClipPath = null
        this.metadata = null
        this.clipUuid = null
        this.stagedClipUuid = null
    }

    saveClip() {
        if (!this.localClipPath || (!this.useServerSideClipping && (!this.metadata || !this.vod))) {
            return
        }

        this.saveInProgress = true
        this.sendAnalyticsEvent(this.AnalyticsCategory.MatchVod, this.AnalyticsAction.SaveClip, '', this.clipEnd - this.clipStart)

        if (this.useServerSideClipping && this.canDoServerSideClipping) {
            // At this point we just need to publish the clip and use the user's title and description.
            apiClient.publishClip(this.stagedClipUuid!, this.clipTitle, this.clipDescription).then(() => {
                this.clipUuid = this.stagedClipUuid
            }).catch((err: any) => {
                this.clipError = true
                console.error('Failed to publish clip: ', err)
            }).finally(() => {
                this.saveInProgress = false
            })
        } else if (!!this.vod && !!this.metadata) {
            apiClient.createClip(this.videoUuid, this.localClipPath, {
                matchUuid: this.vod.matchUuid,
                userUuid: this.$store.state.currentUser.uuid,
                videoUuid: '',
                startTime: new Date(this.vod.startTime.getTime() + this.clipStart),
                endTime: new Date(this.vod.startTime.getTime() + this.clipEnd),
                rawContainerFormat: 'mp4',
                isClip: true,
                isLocal: false,
                expirationTime: null,
            }, this.metadata, this.clipTitle, this.clipDescription, this.game).then((resp: ApiData<string>) => {
                this.clipUuid = resp.data
            }).catch((err: any) => {
                this.clipError = true
                console.error('Failed to create clip: ', err)
            }).finally(() => {
                this.saveInProgress = false
            })
        }
    }

    goToClip() {
        ipcRenderer.send('redirect-to-route', this.clipPathTo)
        ipcRenderer.send('close-vod-editor')
        this.cancelClip()
    }

    returnToMatch() {
        ipcRenderer.send('close-vod-editor')
        this.cancelClip()
    }

    createNewClip() {
        this.cancelClip()
    }

    get clipPathTo(): any {
        return {
            name: pi.ClipPageId,
            params: {
                clipUuid: this.clipUuid
            }
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

        
        let clipStart = Math.round(this.clipStart)
        let clipEnd = Math.round(this.clipEnd + 1000)

        if (clipStart === null || clipEnd === null) {
            console.log(`Encountered a bad clip state [Start: ${clipStart} to End: ${clipEnd}]`)
            console.log(`\tClip Duration: ${this.clipDuration}`)
            console.log(`\tVod Start ${this.start} End ${this.end}`)
            console.log('\tFull VOD Info: ', this.vod, this.metadata)
            this.badClipState = true
            return
        }

        this.clipInProgress = true
        this.showHideClipDialog = true

        this.sendAnalyticsEvent(this.AnalyticsCategory.MatchVod, this.AnalyticsAction.CreateClip, '', clipEnd - clipStart)
        if (this.useServerSideClipping && this.canDoServerSideClipping) {
            // Request server side clipping. Then wait until the clipping is completed by our servers.
            // This creates an un-published clip which we should publish in the next step.
            apiClient.requestServerSideClipping(this.videoUuid, clipStart, clipEnd).then((resp: ApiData<number>) => {
                let stagedId = resp.data
                new Promise<StagedClipStatusResponse>(async (resolve, reject) => {
                    try {
                        while (true) {
                            let status = (await apiClient.checkServerSideClippingStatus(stagedId)).data
                            if (!!status) {
                                resolve(status)
                                break
                            }

                            await new Promise(r => setTimeout(r, 1000 + Math.random() * 2000));
                        }
                    } catch (ex) {
                        reject(ex)
                    }
                }).then((clipStatus: StagedClipStatusResponse) => {
                    this.localClipPath = clipStatus.url
                    this.stagedClipUuid = clipStatus.uuid
                }).catch((err: any) => {
                    console.error('Failed to clip [pending ssc]: ', err)
                    this.clipError = true
                    this.showHideClipDialog = false
                }).finally(() => {
                    this.clipInProgress = false
                })
            }).catch((err: any) => {
                console.error('Failed to clip [request ssc]: ', err)
                this.clipError = true
                this.showHideClipDialog = false
                this.clipInProgress = false
            })
        } else {
            requestVodClip(videoUri, clipStart, clipEnd).then((resp: {
                path: string,
                metadata: VodMetadata,
            }) => {
                let normalPath = resp.path.replace(/\\/g, '/')
                this.clipInProgress = false
                this.clipKey += 1
                this.localClipPath = `file:///${normalPath}`
                this.clipPathsInSession.push(normalPath)
                this.metadata = resp.metadata
            }).catch((err: any) => {
                console.error('Failed to clip: ', err)
                this.clipError = true
                this.showHideClipDialog = false
                this.clipInProgress = false
            })
        }
    }
}

</script>

<style scoped>

#editor-vod {
    height: 75vh;
    max-height: 75vh;
}

.editor-timeline {
    height: 5vh;
    max-height: 5vh;
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

.import-button {
    transform: rotateZ(90deg);
}

</style>