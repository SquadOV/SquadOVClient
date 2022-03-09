<template>
    <div @mouseover="startPlay" @mouseout="pausePlay" :key="forceRedraw"> 
        <video class="video-js vjs-fill" ref="video">
        </video>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component, {mixins} from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import * as vod from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import videojs from 'video.js'
import 'video.js/dist/video-js.css' 

import CommonComponent from '@client/vue/CommonComponent'
/// #if DESKTOP
import { ipcRenderer } from 'electron'
import { IpcResponse } from '@client/js/system/ipc'
/// #endif

@Component
export default class VideoPreviewPlayer extends mixins(CommonComponent) {
    @Prop({required: true})
    vod!: vod.VodManifest

    @Prop({type: Boolean, default: false})
    useLocalVod!: boolean

    @Prop()
    accessToken!: string | null | undefined

    videoUri: string | null = null
    player: videojs.Player | null = null
    previousTime: number = 10000000000
    $refs!: {
        video: HTMLVideoElement
    }

    canPause: boolean = false
    forceRedraw: number = 0

    get hasPreview(): boolean {
        return !!this.videoUri
    }

    @Watch('isActive')
    onActiveChange() {
        this.forceRedraw += 1
        if (this.isActive) {
            Vue.nextTick(() => {
                this.refreshPreviewUri()
            })
        } else if (!!this.player) {
            this.player.dispose()
            this.player = null
        }
    }

    startPlay() {
        if (!!this.player) {
            let el = <HTMLElement>this.player.tech(0).el();
            el.style.opacity = '1.0'

            this.canPause = false
            let promise = this.player.play()
            if (!!promise) {
                promise.finally(() => {
                    this.canPause = true
                })
            } else {
                this.canPause = true
            }
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
        this.canPause = false
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

        this.player.on('canplay', () => {
            this.canPause = true
        })

        this.player.volume(0)
    }

    @Watch('vod')
    refreshPreviewUri() {
        if (this.useLocalVod) {
            ipcRenderer.invoke('check-vod-local', this.vod.videoTracks[0].metadata.videoUuid).then((resp: IpcResponse<string>) => {
                this.videoUri = resp.data
                Vue.nextTick(() => {
                    this.onVideoUriChange()
                })
            }).catch((err: any) => {
                console.error('Failed to get local VOD for preview: ', err)
            })
        } else {
            let previewUri = this.vod.videoTracks[0]?.preview
            if (!previewUri) {
                this.videoUri = null
                return
            }

            apiClient.accessToken(this.accessToken).getVodSegment(previewUri).then((resp : ApiData<vod.VodSegmentUrl>) => {
                this.videoUri = resp.data.url
                if (!!resp.data.expiration) {
                    window.setTimeout(() => {
                        this.refreshPreviewUri()
                    }, Math.max(resp.data.expiration.getTime() - new Date().getTime(), 100))
                }

                Vue.nextTick(() => {
                    this.onVideoUriChange()
                })
            }).catch((err : any) => {
                console.error('Failed to get final URL for video preview: ', err)
            })
        }
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