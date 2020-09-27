<template>
    <div id="vod-container">
        <video v-if="hasVideo" class="video-js vjs-fluid" ref="video">
        </video>

        <v-row class="full-parent-height" justify="center" align="center" v-else>
            <span class="text-h4">No VOD Available.</span>
        </v-row>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import videojs from 'video.js'
import fs from 'fs'
import 'video.js/dist/video-js.css' 

@Component
export default class VideoPlayer extends Vue {
    @Prop({required: true})
    videoFilename! : string | null

    @Prop()
    playerHeight!: number

    player: videojs.Player | null = null

    $refs!: {
        video: HTMLElement
    }

    get prefixedFilename() : string {
        if (!this.videoFilename) {
            return ''
        }
        return `file:///${this.videoFilename}`
    }

    get hasVideo() : boolean {
        if (!this.videoFilename) {
            return false
        }
        return fs.existsSync(this.videoFilename)
    }

    goToTimeMs(tmMs : number) {
        if (!this.player) {
            return
        }

        this.player.currentTime(Math.max(Math.floor(tmMs / 1000.0) - 1.0, 0.0))
    }

    @Watch('hasVideo')
    toggleHasVideo() {
        if (!this.hasVideo) {
            this.player = null
            return
        }

        this.player = videojs(this.$refs.video, {
            controls: true,
            autoplay: false,
            preload: 'auto',
            sources: [
                {
                    src: this.prefixedFilename,
                    type: 'video/mp4',
                },
            ]
        })

        this.player.on('playerresize', () => {
            this.$emit('update:playerHeight', this.player!.currentHeight())
        })

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

    mounted() {
        this.toggleHasVideo()
    }
}

</script>

<style scoped>

#vod-container {
    width: 100%;
    height: 100%;
}

</style>