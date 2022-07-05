<template>
    <div style="position: relative;" @mouseover="startPlay" @mouseout="pausePlay" :key="forceRedraw">
        <video class="video-js vjs-fill stack video" ref="video">
        </video>

        <v-img class="stack thumbnail" :src="thumbnailUrl">
        </v-img>
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

    @Prop()
    vodUuid!: string | undefined

    videoUri: string | null = null
    thumbnailUrl: string | null = null
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

    transitionInVideoPlayer() {
        if (!!this.player) {
            let el = <HTMLElement>this.player.tech(0).el().parentElement;
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

    retrieveCloudVideoUri() {
        // Load up the preview video URI first before we transition in.
        let previewUri = this.vod.videoTracks[0]?.preview
        if (!previewUri) {
            this.videoUri = null
            return
        }

        apiClient.accessToken(this.accessToken).getVodSegment(previewUri).then((resp : ApiData<vod.VodSegmentUrl>) => {
            this.videoUri = resp.data.url
            this.onVideoUriChange()
            if (!!resp.data.expiration) {
                window.setTimeout(() => {
                    this.retrieveCloudVideoUri()
                }, Math.max(resp.data.expiration.getTime() - new Date().getTime(), 100))
            }

            this.transitionInVideoPlayer()
        }).catch((err : any) => {
            console.error('Failed to get final URL for video preview: ', err)
        })
    }

    startPlay() {
        if (this.$store.state.settings.useStaticThumbnails) {
            return
        }

        if (!this.useLocalVod && !this.videoUri) {
            this.retrieveCloudVideoUri()
        } else {
            this.transitionInVideoPlayer()
        }
    }

    pausePlay() {
        if (!!this.player) {
            let el = <HTMLElement>this.player.tech(0).el().parentElement;

            if (this.useLocalVod) {
                el.style.opacity = '1.0'
            } else {
                el.style.opacity = '0.0'
            }

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
            console.log('Using Video Preview: ', this.videoUri)
            this.player.src([
                {
                    src: this.videoUri,
                    type: 'video/mp4',
                },
            ])
        }

        let el = <HTMLElement>this.player.tech(0).el().parentElement
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
    @Watch('vodUuid')
    @Watch('$store.state.settings.useStaticThumbnails')
    refreshPreviewUri() {
        if (!this.useLocalVod) {
            let tbUri = this.vod.videoTracks[0]?.thumbnail
            if (!tbUri) {
                this.thumbnailUrl = null
                return
            }

            apiClient.accessToken(this.accessToken).getVodSegment(tbUri).then((resp : ApiData<vod.VodSegmentUrl>) => {
                this.thumbnailUrl = resp.data.url
                if (!!resp.data.expiration) {
                    window.setTimeout(() => {
                        this.refreshPreviewUri()
                    }, Math.max(resp.data.expiration.getTime() - new Date().getTime(), 100))
                }

                // Create the video player and then pause it to hide it.
                this.onVideoUriChange()
                this.pausePlay()
            }).catch((err : any) => {
                console.error('Failed to get final URL for video thumbnail: ', err)
            })
        } else {
            ipcRenderer.invoke('check-vod-local', this.vodUuid || this.vod.videoTracks[0].metadata.videoUuid).then((resp: IpcResponse<string>) => {
                this.videoUri = resp.data
                Vue.nextTick(() => {
                    this.onVideoUriChange()
                })
            }).catch((err: any) => {
                console.error('Failed to get local VOD for preview: ', err)
            })
        }
    }

    mounted() {
        Vue.nextTick(() => {
            this.refreshPreviewUri()
        })
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

.stack {
    position: absolute;
    top: 0;
    left: 0;
}

.stack.video {
    z-index: 3;
}

.stack.thumbnail {
    z-index: 2;
    width: 100%;
    height: 100%;
}
</style>