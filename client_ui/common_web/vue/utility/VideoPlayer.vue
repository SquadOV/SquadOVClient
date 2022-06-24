<template>
    <div>
        <div v-if="hasVideo" :style="parentDivStyle" :key="forceRedraw">
            <video :class="`video-js ${!fill ? 'vjs-fluid': 'vjs-fill'}`" ref="video">
            </video>

            <video-draw-overlay
                v-if="enableDraw"
                ref="overlay"
            >
            </video-draw-overlay>
            <video-end-overlay
                v-if="showEndOverlay"
                @replayVOD="replayVOD"
                @closeOverlay="closeOverlay"
                ref="endOverlay"
                :clip-uuid="clipUuid"
                :permissions="permissions"
                :full-path="$route.fullPath"
                :timestamp="timestamp"
                no-clip
                :match-uuid="matchUuid"
                :game="game"
                :user-id="userId"
                :is-local="isLocal"
                :graphql-stats="graphqlStats"
                >
            </video-end-overlay>
        </div>

        <v-row class="empty-container" justify="center" align="center" v-else>
            <v-col cols="7">
                <div class="text-h4 font-weight-bold">No VOD Available.</div>
                <v-alert border="left" colored-border color="error" elevation="2" class="text-subtitle-2 mt-2 py-2">
                    SquadOV must be running on your machine (not the web client) for it to record a VOD. We <b>DO NOT</b> record VODs for games that happened in the past or when SquadOV is not running/installed.
                    Please refer to the support wizard on how to properly setup SquadOV for the game(s) you play.
                    Sometimes it takes a bit longer for your machine to upload VODs to our servers, in that case please wait a few minutes.
                    If all else fails, please feel free to submit a bug report!
                </v-alert>
            </v-col>
        </v-row>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component, {mixins} from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import * as vod from '@client/js/squadov/vod'
import videojs from 'video.js'
import 'video.js/dist/video-js.css' 
import { Parser as M3u8Parser } from 'm3u8-parser'
import VideoDrawOverlay from '@client/vue/utility/VideoDrawOverlay.vue'
import { RCMessagePacket, RCMessageType, VodRemoteControlContext } from '@client/js/vods/remote'
import playerjs from 'player.js'
import VideoEndOverlay from '@client/vue/utility/VideoEndOverlay.vue'

/// #if DESKTOP
import { ipcRenderer } from 'electron'
import { IpcResponse } from '@client/js/system/ipc'
/// #endif

import CommonComponent from '@client/vue/CommonComponent'
import { PlayerPageId } from '@client/js/pages'
import { openPathInNewWindow } from '@client/js/external'
import { MatchVideoSharePermissions } from '@client/js/squadov/share'
import { SquadOvGames } from '@client/js/squadov/game'
import { StatPermission } from '@client/js/stats/statPrimitives'
import { pl } from 'date-fns/locale'

const AudioTrackMenuItem = videojs.getComponent('AudioTrackMenuItem')
const MenuItem = videojs.getComponent('MenuItem')

// I'm not sure why this is needed - it didn't seem to work out of the box.
// Maybe we're on an older version of VideoJS that's bugged?
class CustomAudioTrackMenuItem extends AudioTrackMenuItem {
    handleClick(event: any) {
        //@ts-ignore
        MenuItem.prototype.handleClick.call(this, event)

        this.player_.trigger('preChangeAudioTrack')

        //@ts-ignore
        this.track.enabled = true
        //@ts-ignore
        const tracks = this.player_.audioTracks()

        for (let i = 0; i < tracks.length; i++) {
            const track = tracks[i]

            //@ts-ignore
            if (track === this.track) {
                continue
            }

            //@ts-ignore
            track.enabled = track === this.track
        }
    }
}

@Component({
    components: {
        VideoDrawOverlay,
        VideoEndOverlay,
    }
})
export default class VideoPlayer extends mixins(CommonComponent) {
    //Props to pass down to Overlay
    @Prop()
    matchUuid!: string | undefined

    @Prop()
    clipUuid!: string | undefined

    @Prop()
    game!: SquadOvGames

    @Prop({required: true})
    permissions!: MatchVideoSharePermissions | null

    @Prop({type: Boolean, default: false})
    noClip!: boolean

    @Prop()
    graphqlStats!: StatPermission[]

    @Prop({required: true})
    fullPath!: string

    @Prop({default: 0})
    timestamp!: number

    @Prop()
    userId!: number

    @Prop({type: Boolean, default: false})
    isLocal!: boolean

    // End of props that need to be passed down to VideoEndOverlay

    @Prop({required: true})
    vod! : vod.VodAssociation | null | undefined
    
    @Prop()
    overrideUri!: string | undefined

    @Prop({type: Boolean, default: false})
    enableDraw!: boolean

    // Our custom manifest file format that lists all the available options
    // for video quality as well as the urls to get that particular
    // file.
    manifest: vod.VodManifest | null = null
    videoUri: string | null = null

    // When we load a new video, this is the time we want to go to immediately.
    pinnedTimeStamp: Date | null = null
    pinnedPlaying: boolean = false
    reactivateTimestamp: Date | null = null

    @Prop()
    playerHeight!: number

    @Prop({type : Boolean, default: false})
    disableTheater! : boolean

    @Prop({type : Boolean, default: false})
    disablePopout! : boolean

    @Prop()
    currentTime!: Date | null

    @Prop()
    currentTs!: number

    @Prop()
    ready!: boolean | undefined

    @Prop({type: Boolean, default: false})
    fill!: boolean

    @Prop({type: Boolean, default: false})
    loopClip!: boolean

    @Prop()
    clipStart!: number | undefined

    @Prop()
    clipEnd!: number | undefined

    @Prop({default: 3000})
    goToOffset!: number

    @Prop({type: Boolean, default: false})
    disableShortcuts!: boolean

    player: videojs.Player | null = null
    $refs!: {
        video: HTMLVideoElement
        overlay: VideoDrawOverlay
        endOverlay: VideoEndOverlay
    }
    hasMadeProgress: boolean = false
    rcContext: VodRemoteControlContext | null = null
    forceNoVideo: boolean = false
    forceRedraw: number = 0
    syncedInputTs: boolean = false
    lastTimestamp: number = 0
    refreshInterval: number | null = null
    currentWatchRangeSeconds: number | null = null
    showEndOverlay = false
    cumulativeWatchTime: number = 0

    audioTrackCacheTime: number = 0

    closeOverlay() {
        this.showEndOverlay = false
    }

    startWatchAnalyticsRange() {
        if (!this.player || this.currentWatchRangeSeconds !== null) {
            return
        }

        this.currentWatchRangeSeconds = this.player.currentTime()
    }

    stopWatchAnalyticsRange(videoUuid: string | undefined = undefined, manualStopTime: number | undefined = undefined) {
        if (!this.player || this.currentWatchRangeSeconds === null) {
            return
        }

        let start = this.currentWatchRangeSeconds
        let end = (manualStopTime !== undefined) ? manualStopTime : this.player.currentTime()

        if (end <= start) {
            if (end != start) {
                console.warn('Invalid range...start after end: ', start, end)
            }
            return
        }

        let finalVideoUuid = !!videoUuid ? videoUuid : this.vod?.videoUuid
        if (!!finalVideoUuid) {
            apiClient.markUserVideoWatchRange(finalVideoUuid, start, end).catch((err: any) => {
                console.warn('Failed to mark user video watch range: ', err)
            })
        }

        this.currentWatchRangeSeconds = null
    }

    get parentDivStyle(): any {
        if (this.fill) {
            return {
                'width': '100%',
                'height': '100%',
            }
        } else {
            return {}
        }
    }

    get hasVideo() : boolean {
        if ((!this.vod && !this.overrideUri) || this.forceNoVideo) {
            return false
        }
        return true
    }


    handleRcPacket(p: RCMessagePacket) {
        switch (p.type) {
            case RCMessageType.StopAndDestroy:
                if (!!this.player) {
                    this.forceNoVideo = true
                    this.player.dispose()
                    this.player = null
                }
                break
            case RCMessageType.GoToTimestamp:
                this.goToTimeMs(p.data, false, false)
                break
        }
    }

    @Watch('isActive')
    onActiveChange() {
        if (this.isActive && !!this.player && !!this.vod) {
            this.reactivateTimestamp = new Date(this.vod.startTime.getTime() + this.player.currentTime() * 1000)
        } else if (!this.isActive) {
            this.setNoVideo()
            this.stopWatchAnalyticsRange()
        }

        this.forceRedraw += 1
    }

    @Watch('vod')
    refreshPlaylist(newVod: vod.VodAssociation | null | undefined, oldVod: vod.VodAssociation | null | undefined) {
        this.manifest = null
///#if DESKTOP
        if (!!this.rcContext) {
            this.rcContext.close()
        }
///#endif

        this.rcContext = null
        this.forceNoVideo = false
        if (!this.hasVideo) {
            this.setNoVideo()
            return
        }

///#if DESKTOP
        this.rcContext = new VodRemoteControlContext(this.vod!.videoUuid)
        this.rcContext.addListener((p: RCMessagePacket) => {
            this.handleRcPacket(p)
        })
///#endif

        // Generally the only time we'll be switching videoUuids is when we go from one
        // user's POV to another. Therefore, we're able to make the assumption that the
        // person watching the VOD would want the new VOD to go to the same point in time as the
        // old VOD. Note that readyState is:
        //    HAVE_NOTHING (numeric value 0) No information regarding the media resource is available.
        //    HAVE_METADATA (numeric value 1) Enough of the resource has been obtained that the duration of the resource is available.
        //    HAVE_CURRENT_DATA (numeric value 2) Data for the immediate current playback position is available.
        //    HAVE_FUTURE_DATA (numeric value 3) Data for the immediate current playback position is available, as well as enough data for the user agent to advance the current playback position in the direction of playback.
        //    HAVE_ENOUGH_DATA (numeric value 4) The user agent estimates that enough data is available for playback to proceed uninterrupted.

        if (!!oldVod && !!this.player && this.player.readyState() >= 2) {
            this.stopWatchAnalyticsRange(oldVod.videoUuid, undefined)
            let desiredVideoTime = this.player.currentTime()
            this.pinnedTimeStamp = new Date(oldVod.startTime.getTime() + desiredVideoTime * 1000)
            this.pinnedPlaying = !this.player.paused()
        } else if (!!this.reactivateTimestamp) {
            this.pinnedTimeStamp = this.reactivateTimestamp
            this.pinnedPlaying = false
            this.reactivateTimestamp = null
        } else {
            this.pinnedTimeStamp = null
            this.pinnedPlaying = false
        }

///#if DESKTOP
        this.videoUri = null
        ipcRenderer.invoke('check-vod-local', this.vod!.videoUuid).then((resp: IpcResponse<string>) => {
            if (resp.success) {
                console.log('Found Local VOD: ', resp.data)
                if (this.videoUri !== resp.data) {
                    this.videoUri = resp.data
                } else {
                    // Same video? Probably due to keep-alive when the user switched back to the page.
                    // Need to make sure the video refreshes.
                    this.refreshPlayerSources()
                }
                this.toggleHasVideo()
            } else {
                console.log('No Local VOD for: ', this.vod!.videoUuid)
            }
        }).catch((err: any) => {
            console.log('Failed to get local VOD...falling back to checking SquadOV server:', err)
        }).finally(() => {
            if (!this.videoUri) {
///#endif
                apiClient.accessToken().getVodManifest(this.vod!.videoUuid!).then((resp : ApiData<vod.VodManifest>) => {
                    this.manifest = resp.data
                    this.toggleHasVideo()
                }).catch((err : any) => {
                    this.setNoVideo()
                    console.error('Failed to obtain VOD manifest: ', err)
                })
///#if DESKTOP
            }
        })
///#endif
    }

    goToPercentage(percent: number) {
        if (!this.player) {
            return
        }
        let time = this.player.duration() * percent
        this.goToTimeMs(time * 1000.0, false)
    }

    goToTimeMs(tmMs : number, useOffset: boolean, propagate: boolean = true) {
        if (!this.player || this.player.readyState() < 3) {
            return
        }

        let newTime = Math.max(Math.floor((tmMs - (useOffset ? this.goToOffset : 0.0)) / 1000.0), 0.0)
        if (!propagate) {
            // We need to set lastTimestamp here since we don't want a large jump to propagate
            // another goToTimestamp packet to be sent out via the large time jump detection
            // in the 'timeupdate' event.
            this.lastTimestamp = newTime
        }

        this.player.currentTime(newTime)

        if (propagate) {
///#if DESKTOP
            this.rcContext?.goToTimestamp(newTime * 1000.0)
///#endif
        }

    }

    get currentVideoSourceUri() : string {
        if (!this.manifest) {
            return ''
        }

        let track = vod.getVideoQualityTrack(this.manifest!, 'source')
        if (!track) {
            return ''
        }
        return track.segments[0].uri
    }

    get vodLengthSeconds(): number {
        if (!this.vod) {
            return 0
        }
        return (this.vod.endTime.getTime() - this.vod.startTime.getTime()) / 1000
    }

    get currentMimeType(): string {
        if (!this.manifest) {
            return 'video/mp4'
        }

        let track = vod.getVideoQualityTrack(this.manifest!, 'source')
        if (!track) {
            return 'video/mp4'
        }
        return track.segments[0].mimeType
    }

    @Watch('currentVideoSourceUri')
    refreshVideoSource() {
        if (this.currentVideoSourceUri == '') {
            this.videoUri = null
            return
        }

        apiClient.accessToken().getVodSegment(this.currentVideoSourceUri).then((resp : ApiData<vod.VodSegmentUrl>) => {
            this.videoUri = resp.data.url
            if (!!resp.data.expiration) {
                this.refreshInterval = window.setTimeout(() => {
                    this.refreshInterval = null
                    this.refreshVideoSource()
                }, Math.max(resp.data.expiration.getTime() - new Date().getTime(), 100))
            }
        }).catch((err : any) => {
            console.error('Failed to get final URL for video.')
        })
    }

    @Watch('videoUri')
    refreshPlayerSources() {
        if (!this.player) {
            return
        }

        if (!!this.videoUri) {
            this.$emit('update:ready', false)
            console.log('Refresh Player Source: ', this.videoUri, this.currentMimeType)
            if (this.currentMimeType == 'video/mp2t') {
                // Need to generate a manifest to get video.js to play a mpeg-ts file.
                const parser = new M3u8Parser();
                parser.push([
                    '#EXTM3U',
                    '#EXT-X-VERSION:3',
                    `#EXT-X-TARGETDURATION:${this.vodLengthSeconds}`,
                    '#EXT-X-MEDIA-SEQUENCE:0',
                    '#EXT-X-DISCONTINUITY-SEQUENCE:0',
                    `#EXTINF:${this.vodLengthSeconds}`,
                    this.videoUri,
                    '#EXT-X-ENDLIST'
                ].join('\n'));
                parser.end();

                this.player.src({
                    src: `data:application/vnd.videojs.vhs+json,${JSON.stringify(parser.manifest)}`,
                    type: 'application/vnd.videojs.vhs+json'
                });
            } else {
                this.player.src([
                    {
                        src: this.videoUri,
                        type: this.currentMimeType,
                    },
                ])
            }
        }
    }

    setNoVideo() {
        this.$emit('update:playerHeight', 500)
        if (!!this.player) {
            this.player.dispose()
            this.player = null
        }
        this.forceNoVideo = true
    }

    setPinned(dt: Date) {
        this.pinnedTimeStamp = dt
    }

    // This should only happen once. Manifest updates should be handled by the media source.
    toggleHasVideo() {
        if (!!this.player) {
            return
        }

        this.forceNoVideo = false
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
        }, () => {
            const adapter = new playerjs.VideoJSAdapter(this.player)
            adapter.ready()
        })

        // Just in case videoUri and audioUri got set earlier.
        this.refreshPlayerSources()
        this.hasMadeProgress = true
        this.player.on('playerresize', () => {
            this.$emit('update:playerHeight', this.player!.currentHeight())
        })

        // The videojs player can change height on the canplay event and it doesn't
        // trigger any of the *resize events.
        this.player.on('canplay', () => {
            this.$emit('update:playerHeight', this.player!.currentHeight())
            if (!!this.vod && !!this.player && this.player.readyState() >= 2) {
                let setTime = false
                if (!!this.pinnedTimeStamp) {
                    console.log('go to pinned?: ', this.pinnedTimeStamp, this.vod.startTime)
                    this.goToTimeMs(this.pinnedTimeStamp.getTime() - this.vod.startTime.getTime(), false)
                    this.pinnedTimeStamp = null
                    setTime = true
                } else if (!this.syncedInputTs) {
                    this.syncedInputTs = true
                    this.player.currentTime(this.currentTs)
                    setTime = true
                }

                if (this.pinnedPlaying && setTime) {
                    this.player.play()
                }
            }
            this.$store.commit('successfullyVisitedVideo', true)
            this.$emit('update:ready', true)
        })

        this.player.on('timeupdate', () => {
            if (!!this.vod && !!this.player) {
                if (this.loopClip && this.clipEnd !== undefined && this.clipStart !== undefined) {
                    if (this.hasMadeProgress && (this.player.currentTime() > this.clipEnd || this.player.currentTime() < this.clipStart)) {
                        this.player.currentTime(this.clipStart)
                        this.hasMadeProgress= false
                    }
                }

                let newCurrentTime = new Date(this.vod.startTime.getTime() + this.player.currentTime() * 1000)
                this.hasMadeProgress = !this.currentTime || (newCurrentTime > this.currentTime)

                // We don't particularly need to update this very often so cap it at showing 1s differences.
                let diff = (!!this.currentTime ? Math.abs(this.currentTime.getTime() - newCurrentTime.getTime()) : 1000000000000000000) / 1000.0
                if (diff > 1.0) {
                    this.$emit('update:currentTime', newCurrentTime)
                }

                this.$emit('update:currentTs', this.player.currentTime())

                // I'd rather lastTimestamp be an indicator of when the player was actually playing
                // so we have a better sense of what the pre-seek timestamp was for analytics.
                let diffFromLastTs = Math.abs(this.player.currentTime() - this.lastTimestamp)
                if (diffFromLastTs > 1.0) {
                    this.rcContext?.goToTimestamp(this.player.currentTime() * 1000.0)
                    this.lastTimestamp = this.player.currentTime()

                    if (!this.player.seeking() && !this.player.paused()) {
                        this.cumulativeWatchTime += diffFromLastTs

                        if (!!this.player && this.cumulativeWatchTime > 180.0) {
                            // Our Google analytics session timeout is 5 minutes long.
                            // So once we reached a good portion of that we want to send an update
                            // that the session is still active because the user is still watching
                            // a VOD/clip.
                            this.sendAnalyticsEvent(this.AnalyticsCategory.MatchVod, this.AnalyticsAction.VodTimeUpdate, '', this.player.currentTime())
                            this.cumulativeWatchTime = 0.0
                        }
                    }
                }
            }
        })

        this.player.on('ended', () => {
            this.showEndOverlay = true
        })

        this.player.on('play', () => {
            this.showEndOverlay = false
            if (!!this.player) {
                this.sendAnalyticsEvent(this.AnalyticsCategory.MatchVod, this.AnalyticsAction.PlayVod, '', this.player.currentTime())
            }
            this.startWatchAnalyticsRange()
        })

        this.player.on('pause', () => {
            // Pause events are sent when seeking when the video is playing.
            if (!!this.player?.seeking()) {
                return
            }

            if (!!this.player) {
                this.sendAnalyticsEvent(this.AnalyticsCategory.MatchVod, this.AnalyticsAction.StopVod, '', this.player.currentTime())
            }
            this.stopWatchAnalyticsRange()
        })

        this.player.on('seeking', () => {
            // If the user did a manual jump then we also need to manually close out the previous range
            // since otherwise when the user stops playing the VOD we'll get a much longer range (e.g. if they jumped
            // forward 5 minutes that's an extra 5 minutes of time we add to their watch time). Note that we can't use the
            // player's currentTime since that'll be inaccurate (it'll be the time we're jumping to) so we use the lastTimestamp instead.
            this.stopWatchAnalyticsRange(undefined, this.lastTimestamp)

            // If the user is still playing the VOD then start a new range.
            if (!this.player?.paused()) {
                this.startWatchAnalyticsRange()
            }
        })

        this.player.on('fullscreenchange', () => {
            if (!!this.$refs.overlay) {
                this.$refs.overlay.overlayResize()
            }
        })

        this.player.on('volumechange', () => {
            if (!this.player) {
                return
            }

            let volume = this.player.muted() ? 0 : this.player.volume()
            window.sessionStorage.setItem('squadOvVideoPlayerVolume', JSON.stringify(volume))
        })

        let savedVolume = window.sessionStorage.getItem('squadOvVideoPlayerVolume')
        if (!!savedVolume) {
            this.player.volume(JSON.parse(savedVolume))
        }

        let controlBar = this.player.getChild('controlBar')!
        let button = videojs.getComponent('Button')
        if (!this.disableTheater) {
            // Construct a custom "theater mode" button a la YouTube 
            //@ts-ignore
            let theaterModeButtonCls = videojs.extend(button, {
                constructor: function() {
                    //@ts-ignore
                    button.apply(this, arguments)
                    this.addClass('mdi')
                    this.addClass('mdi-fix')
                    this.addClass('mdi-rectangle-outline')
                    this.controlText('Toggle Theater Mode')
                },
            })
            videojs.registerComponent('theaterModeButton', theaterModeButtonCls)

            let theaterModeButton = controlBar.addChild('theaterModeButton', {}, 16)
            theaterModeButton.on('click', () => {
                this.$emit('toggle-theater-mode')
            })
        }

///#if DESKTOP
        if (!this.disablePopout) {
            // Construct a popout button that'll cause a new window to pop out that contains JUST the video.
            //@ts-ignore
            let popoutButtonCls = videojs.extend(button, {
                constructor: function() {
                    //@ts-ignore
                    button.apply(this, arguments)
                    this.addClass('mdi')
                    this.addClass('mdi-fix')
                    this.addClass('mdi-open-in-new')
                    this.controlText('Popout Video to new Window')
                },
            })
            videojs.registerComponent('popoutButton', popoutButtonCls)

            let popoutButton = controlBar.addChild('popoutButton', {}, 16)
            popoutButton.on('click', () => {
                let path = this.$router.resolve({
                    name: PlayerPageId,
                    params: {
                        vodUuid: this.vod!.videoUuid
                    }
                })
                openPathInNewWindow(path.href)
            })
        }
///#endif

        let cbar = this.player.getChild('controlBar')
    
        if (!!cbar) {
            // Remove the audio track control button since VideoJS's button doesn't really work.
            let atButton = cbar.getChild('audioTrackButton')
            if (!!atButton) {
                cbar.removeChild(atButton)
            }

///#if DESKTOP
            // Only add back our custom button on the desktop since we can't control whether or not
            // the user is 1) in chrome and 2) has the experimental web features enabled.
            //@ts-ignore
            let AudioTrackButtonComponent = videojs.extend(videojs.getComponent('AudioTrackButton'), {
                createItems: function(items: any[] = []) {
                    this.hideThreshold_ = 1;

                    const tracks = this.player_.audioTracks()

                    for (let i = 0; i < tracks.length; i++) {
                        const track = tracks[i]

                        items.push(new CustomAudioTrackMenuItem(this.player_, {
                            //@ts-ignore
                            track,
                            selectable: true,
                            multiSelectable: false
                        }))
                    }

                    return items;
                }
            })
            videojs.registerComponent('audioTrackButtonALT', AudioTrackButtonComponent)
            controlBar.addChild('audioTrackButtonALT', {}, 14)

            this.player.on('preChangeAudioTrack', () => {
                if (!this.player) {
                    return
                }

                this.audioTrackCacheTime = this.player.currentTime()
            })

            this.player.tech().audioTracks().addEventListener('change', () => {
                // Do a seek to the current time. For some reason when the audio track changes,
                // the new audio track needs to play from the beginning until it hits the current time.
                // It seems like we can bypass this behavior by doing a seek.
                setTimeout(() => {
                    if (!this.player) {
                        return
                    }
                    this.player.currentTime(this.audioTrackCacheTime)
                }, 1000)
            })

///#endif
            for (let ctrl of cbar.children()) {
                ctrl.on('keydown', (e: KeyboardEvent) => {
                    if (e.key != 'Enter') {
                        // We only want to use the enter key do stuff (toggle the element).
                        // Everything else should just be handled by the player.
                        e.preventDefault()

                        // This needs to be here since the control bar will eat focus and prevent the handle keypress from working normally.
                        this.handleKeypress(e)
                    }
                })
            }

            let playbackButton = cbar.getChild('playbackRateMenuButton')
            if (!!playbackButton) {
                // Custom functionality to decrease the playback rate.
                playbackButton.on('contextmenu', (e: Event) => {
                    if (!this.player || !playbackButton) {
                        return
                    }
                    const currentRate = this.player.playbackRate()

                    //@ts-ignore
                    const rates = playbackButton.playbackRates()
                    const currentIndex = rates.indexOf(currentRate)
                    // this get the next rate and it will select first one if the last one currently selected
                    let newIndex = currentIndex - 1
                    if (newIndex < 0) {
                        newIndex = rates.length - 1
                    }
                    this.player.playbackRate(rates[newIndex])
                    e.preventDefault()
                })
            }
        }
    }

    @Watch('player')
    @Watch('enableDraw')
    refreshDrawDiv() {
        Vue.nextTick(() => {
            if (!this.player || !this.enableDraw) {
                return
            }
            let newParent = this.player.el()
            newParent.appendChild(this.$refs.overlay.$el)
            this.player.pause()
        })
    }

    @Watch('player')
    @Watch('showEndOverlay')
    refreshEndOverlay() {
        Vue.nextTick(() => {
            if (!this.player || !this.showEndOverlay) {
                return
            }
            let newParent = this.player.el()
            newParent.appendChild(this.$refs.endOverlay.$el)
            this.player.pause()
        })
    }

    replayVOD() {
        this.closeOverlay()
        this.player?.play()
    }

    handleKeypress(e: KeyboardEvent) {
        if (!this.player) {
            return
        }

        if (e.ctrlKey || e.altKey) {
            return
        }

        if (this.disableShortcuts) {
            return
        }

        if (!this.isActive) {
            return
        }

        // Two additional cases where the video player shouldn't:
        //  1) if the user is trying to input something into a form.
        //  2) if the user has some dialog open.
        if (document.activeElement?.tagName === 'INPUT' ||
            document.activeElement?.className.includes('v-dialog') || 
            document.activeElement?.tagName === 'TEXTAREA') {
            return
        }

        let cmp = e.key.toLowerCase()
        let handled = false
        if (cmp == ' ' || cmp == 'k') {
            // Play/Pause
            if (this.player.paused()) {
                this.player.play()
            } else {
                this.player.pause()
            }
            handled = true
        } else if (cmp == 'f') {
            // Full-Screen
            if (this.player.isFullscreen()) {
                this.player.exitFullscreen()
            } else {
                this.player.requestFullscreen()
            }
            handled = true
        } else if (cmp == 'arrowleft') {
            // Jump back 5 seconds
            this.goToTimeMs(this.player.currentTime() * 1000 - (!!this.$store.state.settings ? this.$store.state.settings.playback.smallStepSize : 5000), false)
            handled = true
        } else if (cmp == 'arrowright') {
            // Jump forward 5 seconds
            this.goToTimeMs(this.player.currentTime() * 1000 + (!!this.$store.state.settings ? this.$store.state.settings.playback.smallStepSize : 5000), false)
            handled = true
        } else if (cmp == 'j') {
            // Jump back 10 seconds
            this.goToTimeMs(this.player.currentTime() * 1000 - (!!this.$store.state.settings ? this.$store.state.settings.playback.largeStepSize : 10000), false)
            handled = true
        } else if (cmp == 'l') {
            // Jump forward 10 seconds
            this.goToTimeMs(this.player.currentTime() * 1000 + (!!this.$store.state.settings ? this.$store.state.settings.playback.largeStepSize : 10000), false)
            handled = true
        } else if (cmp == 'arrowup') {
            // Volume up
            this.player.volume(Math.min(Math.max(this.player.volume() + 0.05, 0.0), 1.0))
            handled = true
        } else if (cmp == 'arrowdown') {
            // Volume down
            this.player.volume(Math.min(Math.max(this.player.volume() - 0.05, 0.0), 1.0))
            handled = true
        } else if (cmp == 'm') {
            // Mute/un-mute
            if (this.player.muted()) {
                this.player.muted(false)
            } else {
                this.player.muted(true)
            }
            handled = true
        } else if (!isNaN(parseInt(cmp))) {
            // Jump to % in video
            let percent = parseInt(cmp) / 10.0
            this.goToPercentage(percent)
            handled = true
        } else if (cmp == 'home') {
            // Go to beginning
            this.goToPercentage(0.0)
            handled = true
        } else if (cmp == 'end') {
            // Go to near end
            this.goToPercentage(0.99)
            handled = true
        } else if (cmp == 'i') {
            //@ts-ignore
            if (this.player.isInPictureInPicture()) {
                //@ts-ignore
                this.player.exitPictureInPicture()
            } else {
                //@ts-ignore
                this.player.requestPictureInPicture()
            }
        } else if (cmp == 't') {
            this.$emit('toggle-theater-mode')
        }

        if (handled) {
            e.preventDefault()
            e.stopPropagation()
        }
    }

    handleAppActiveChange(event: any, active: boolean) {
        if (!this.player) {
            return
        }

        if (!active) {
            this.player.pause()
        }
    }

    keydownHandler: any = null
    appActiveHandler: any = null

    mounted() {
        this.keydownHandler = this.handleKeypress.bind(this)
        window.addEventListener('keydown', this.keydownHandler)

/// #if DESKTOP
        this.appActiveHandler = this.handleAppActiveChange.bind(this)
        ipcRenderer.on('onActiveChange', this.appActiveHandler)
/// #endif

        if (!!this.overrideUri) {
            this.videoUri = this.overrideUri
            this.toggleHasVideo()
        } else {
            this.refreshPlaylist(this.vod, null)
        }
    }

    beforeDestroy() {
        if (this.refreshInterval !== null) {
            window.clearTimeout(this.refreshInterval)
        }
        
        this.stopWatchAnalyticsRange()
        if (!!this.player) {
            this.player.dispose()
        }

/// #if DESKTOP
        if (!!this.appActiveHandler) {
            ipcRenderer.removeListener('onActiveChange', this.appActiveHandler)
            this.appActiveHandler = null
        }
/// #endif

        window.removeEventListener('keydown', this.keydownHandler)
        this.player = null
    }
}

</script>

<style scoped>

.empty-container {
    height: 500px;
}

>>>.mdi-fix {
    font-size: 24px;
    width: initial;
}

.video-default-view {
    transform: scaleY(0.5);
    transform-origin: center;
}

</style>