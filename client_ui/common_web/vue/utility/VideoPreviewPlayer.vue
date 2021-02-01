<template>
    <div @mouseover="startPlay" @mouseout="pausePlay"> 
        <video class="video-js vjs-fill" ref="video">
        </video>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import * as vod from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import videojs from 'video.js'
import 'video.js/dist/video-js.css' 

@Component
export default class VideoPreviewPlayer extends Vue {
    @Prop({required: true})
    vod!: vod.VodManifest

    videoUri: string | null = null
    player: videojs.Player | null = null
    previousTime: number = 10000000000
    $refs!: {
        video: HTMLVideoElement
    }

    get hasPreview(): boolean {
        return !!this.videoUri
    }

    startPlay() {
        if (!!this.player) {
            let el = <HTMLElement>this.player.tech(0).el();
            el.style.opacity = '1.0'

            this.player.play()
        }
    }

    pausePlay() {
        if (!!this.player) {
            let el = <HTMLElement>this.player.tech(0).el();
            el.style.opacity = '1.0'

            this.player.pause()
        }
    }

    onVideoUriChange() {
        if (!this.player) {
            this.player = videojs(this.$refs.video, {
                controls: false,
                autoplay: false,
                loop: true,
                preload: 'auto',
            })
        }

        // Just in case videoUri and audioUri got set earlier.
        if (!!this.videoUri) {
            this.player.src([
                {
                    src: this.videoUri,
                    type: 'video/mp4',
                },
            ])
        }

        let el = <HTMLElement>this.player.tech(0).el();
        el.style.opacity = '1.0'
        el.classList.add('preview-video')

        this.player.on('timeupdate', () => {
            let tm = this.player!.currentTime()
            let duration = this.player!.duration()
            if (tm < this.previousTime) {
                el.style.opacity = '1.0'
            } else if (tm > (duration - 1.0)) {
                el.style.opacity = '0.0'
            }

            this.previousTime = tm
        })
    }

    @Watch('vod')
    refreshPreviewUri() {
        let previewUri = this.vod.videoTracks[0]?.preview
        if (!previewUri) {
            this.videoUri = null
            return
        }

        apiClient.getVodSegment(previewUri).then((resp : ApiData<string>) => {
            this.videoUri = resp.data
            Vue.nextTick(() => {
                this.onVideoUriChange()
            })
        }).catch((err : any) => {
            console.log('Failed to get final URL for video preview: ', err)
        })
    }

    mounted() {
        this.refreshPreviewUri()
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
>>>.vjs-tech { object-fit: cover; }

>>>.preview-video {
    transition: opacity 1s ease-out;
}
</style>