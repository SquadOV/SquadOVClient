<template>
    <div id="vod-container">
        <video v-if="hasVideo" class="video-js vjs-fluid" ref="video">
        </video>

        <v-row class="empty-container" justify="center" align="center" v-else>
            <span class="text-h4">No VOD Available.</span>
        </v-row>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import * as vod from '@client/js/squadov/vod'
import videojs from 'video.js'
import 'video.js/dist/video-js.css' 
import { Parser as M3u8Parser } from 'm3u8-parser'

@Component
export default class VideoPlayer extends Vue {
    @Prop({required: true})
    vod! : vod.VodAssociation | null | undefined

    // Our custom manifest file format that lists all the available options
    // for video quality as well as the urls to get that particular
    // file.
    manifest: vod.VodManifest | null = null
    videoUri: string | null = null

    // When we load a new video, this is the time we want to go to immediately.
    pinnedTimeStamp: Date | null = null
    pinnedPlaying: boolean = false

    @Prop()
    playerHeight!: number

    @Prop({type : Boolean ,default: false})
    disableTheater! : boolean

    @Prop()
    currentTime!: Date | null

    @Prop()
    ready!: boolean | undefined

    player: videojs.Player | null = null
    $refs!: {
        video: HTMLVideoElement
    }

    get hasVideo() : boolean {
        if (!this.vod) {
            return false
        }
        return true
    }

    @Watch('vod')
    refreshPlaylist(newVod: vod.VodAssociation | null | undefined, oldVod: vod.VodAssociation | null | undefined) {
        this.manifest = null
        if (!this.hasVideo) {
            this.setNoVideo()
            return
        }

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
            let desiredVideoTime = this.player.currentTime()
            this.pinnedTimeStamp = new Date(oldVod.startTime.getTime() + desiredVideoTime * 1000)
            this.pinnedPlaying = !this.player.paused()
        } else {
            this.pinnedTimeStamp = null
            this.pinnedPlaying = false
        }

        apiClient.getVodManifest(this.vod!.videoUuid!).then((resp : ApiData<vod.VodManifest>) => {
            this.manifest = resp.data
            this.toggleHasVideo()
        }).catch((err : any) => {
            console.log('Failed to obtain VOD manifest: ', err)
        })
    }

    goToTimeMs(tmMs : number) {
        if (!this.player || this.player.readyState() < 3) {
            return
        }

        this.player.currentTime(Math.max(Math.floor(tmMs / 1000.0), 0.0))
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
            return ''
        }

        let track = vod.getVideoQualityTrack(this.manifest!, 'source')
        if (!track) {
            return ''
        }
        return track.segments[0].mimeType
    }

    @Watch('currentVideoSourceUri')
    refreshVideoSource() {
        if (this.currentVideoSourceUri == '') {
            this.videoUri = null
            return
        }

        apiClient.getVodSegment(this.currentVideoSourceUri).then((resp : ApiData<string>) => {
            this.videoUri = resp.data
        }).catch((err : any) => {
            console.log('Failed to get final URL for video.')
        })
    }

    @Watch('videoUri')
    refreshPlayerSources() {
        if (!this.player) {
            return
        }

        if (!!this.videoUri) {
            this.$emit('update:ready', false)
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
    }

    // This should only happen once. Manifest updates should be handled by the media source.
    toggleHasVideo() {
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

        // Just in case videoUri and audioUri got set earlier.
        this.refreshPlayerSources()
        
        this.player.on('playerresize', () => {
            this.$emit('update:playerHeight', this.player!.currentHeight())
        })

        // The videojs player can change height on the canplay event and it doesn't
        // trigger any of the *resize events.
        this.player.on('canplay', () => {
            this.$emit('update:playerHeight', this.player!.currentHeight())
            if (!!this.pinnedTimeStamp && !!this.vod && !!this.player && this.player.readyState() >= 2) {
                this.goToTimeMs(this.pinnedTimeStamp.getTime() - this.vod.startTime.getTime())
                this.pinnedTimeStamp = null
                if (this.pinnedPlaying) {
                    this.player.play()
                }
            }
            this.$emit('update:ready', true)
        })

        this.player.on('timeupdate', () => {
            if (!!this.vod && !!this.player) {
                let newCurrentTime = new Date(this.vod.startTime.getTime() + this.player.currentTime() * 1000)

                // We don't particularly need to update this very often so cap it at showing 1s differences.
                let diff = (!!this.currentTime ? Math.abs(this.currentTime.getTime() - newCurrentTime.getTime()) : 1000000000000000000) / 1000.0
                if (diff > 1.0) {
                    this.$emit('update:currentTime', newCurrentTime)
                }
            }
        })

        if (!this.disableTheater) {
            // Construct a custom "theater mode" button a la YouTube 
            let button = videojs.getComponent('Button')

            //@ts-ignore
            let theaterModeButtonCls = videojs.extend(button, {
                constructor: function() {
                    //@ts-ignore
                    button.apply(this, arguments)
                    this.addClass('vjs-icon-square')
                },
            })        
            videojs.registerComponent('theaterModeButton', theaterModeButtonCls)

            let controlBar = this.player.getChild('controlBar')!
            let theaterModeButton = controlBar.addChild('theaterModeButton', {}, 16)
            theaterModeButton.on('click', () => {
                this.$emit('toggle-theater-mode')
            })
        }
    }

    mounted() {
        this.refreshPlaylist(this.vod, null)
    }

    beforeDestroy() {
        if (!!this.player) {
            this.player.dispose()
        }
        this.player = null
    }
}

</script>

<style scoped>

#vod-container {
    width: 100%;
    height: 100%;
}

.empty-container {
    height: 500px;
}

</style>