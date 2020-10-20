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
import fs from 'fs'
import 'video.js/dist/video-js.css' 

@Component
export default class VideoPlayer extends Vue {
    @Prop({required: true})
    videoUuid! : string | null

    // Our custom manifest file format that lists all the available options
    // for video quality as well as the urls to get that particular
    // file.
    manifest: vod.VodManifest | null = null
    videoUri: string | null = null

    @Prop()
    playerHeight!: number

    @Prop({type : Boolean ,default: false})
    disableTheater! : boolean

    player: videojs.Player | null = null
    $refs!: {
        video: HTMLVideoElement
    }

    get hasVideo() : boolean {
        if (!this.videoUuid) {
            return false
        }
        return true
    }

    @Watch('videoUuid')
    refreshPlaylist() {
        this.manifest = null
        if (!this.hasVideo) {
            this.setNoVideo()
            return
        }

        apiClient.getVodManifest(this.videoUuid!).then((resp : ApiData<vod.VodManifest>) => {
            this.manifest = resp.data
            this.toggleHasVideo()
        }).catch((err : any) => {
            console.log('Failed to obtain VOD manifest: ', err)
        })
    }

    goToTimeMs(tmMs : number) {
        if (!this.player) {
            return
        }

        this.player.currentTime(Math.max(Math.floor(tmMs / 1000.0) - 1.0, 0.0))
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
            this.player.src([
                {
                    src: this.videoUri,
                    type: 'video/mp4',
                },
            ])
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
        })

        // Just in case videoUri and audioUri got set earlier.
        this.refreshPlayerSources()
        
        this.player.on('playerresize', () => {
            this.$emit('update:playerHeight', this.player!.currentHeight())
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
        this.refreshPlaylist()
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