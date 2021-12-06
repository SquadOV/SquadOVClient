<template>
    <div>
        <router-link :to="clipTo" :event="disableClick ? '' : 'click'">
            <div class="clip-div full-width">
                <div class="mini-clip-preview mb-1">
                    <video-preview-player
                        class="full-width full-parent-height"
                        :vod="clip.manifest"
                        :access-token="accessToken"
                    >
                    </video-preview-player>
                </div>
                
                <div class="d-flex align-center">
                    <div class="flex-grow-1">
                        <div :class="`d-flex align-center ${ hideDetails ? 'justify-center' : '' }`">
                            <v-tooltip bottom>
                                <template v-slot:activator="{on, attrs}">
                                    <v-icon
                                        v-if="!!clip.favoriteReason && !hideDetails"
                                        v-bind="attrs"
                                        v-on="on"
                                        color="warning"
                                        small
                                    >
                                        mdi-star
                                    </v-icon>
                                </template>

                                {{ clip.favoriteReason }}
                            </v-tooltip>
                            <v-img
                                class="mr-1"
                                width="16px"
                                max-width="16px"
                                height="16px"
                                max-height="16px"
                                contain
                                :src="$root.generateAssetUri(gameToIcon(clip.game))"
                            >
                            </v-img>
                            <div class="font-weight-bold text-subtitle-1">{{ clip.title }}</div>

                            <template v-if="!hideDetails">
                                <v-spacer></v-spacer>
                                <div class="text-body-2">{{ clipTime }}</div>
                                <v-icon small v-if="clip.isWatchlist">
                                    mdi-playlist-check
                                </v-icon>
                            </template>
                        </div>

                        <div :class="`d-flex align-center`" v-if="!hideDetails">
                            <div class="text-body-2 font-weight-bold">{{ clip.clipper }}</div>
                            <v-spacer></v-spacer>
                            <v-icon
                                x-small
                                class="mr-1"
                            >
                                mdi-eye
                            </v-icon>
                            <div class="text-body-2 mr-4">{{ clip.views }}</div>
                            
                            <v-icon
                                x-small
                                class="mr-1"
                            >
                                mdi-thumb-up
                            </v-icon>
                            <div class="text-body-2 mr-4">{{ clip.reacts }}</div>

                            <v-icon
                                x-small
                                class="mr-1"
                            >
                                mdi-comment
                            </v-icon>
                            <div class="text-body-2">{{ clip.comments }}</div>
                        </div>
                    </div>
                </div>                
            </div>
        </router-link>
        <bulk-tag-display
            v-if="!hideTags"
            class="mt-1"
            :video-uuid="clip.clip.videoUuid"
            :tags="clip.tags"
            :max-tags="5"
        >
        </bulk-tag-display>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { cleanVodClipFromJson, VodClip } from '@client/js/squadov/vod'
import { standardFormatTime } from '@client/js/time'
import { gameToIcon } from '@client/js/squadov/game'
import VideoPreviewPlayer from '@client/vue/utility/VideoPreviewPlayer.vue'
import BulkTagDisplay from '@client/vue/utility/vods/BulkTagDisplay.vue'
import * as pi from '@client/js/pages'

@Component({
    components: {
        VideoPreviewPlayer,
        BulkTagDisplay,
    }
})
export default class MiniClipPreview extends Vue {
    @Prop({required: true})
    clip!: VodClip

    @Prop({type: Boolean, default: false})
    disableClick!: boolean

    @Prop()
    accessToken!: string | undefined | null

    @Prop({type: Boolean, default: false})
    hideDetails!: boolean

    @Prop({type: Boolean, default: false})
    hideTags!: boolean

    gameToIcon = gameToIcon

    get clipTime(): string {
        return standardFormatTime(this.clip.tm)
    }

    get clipTo(): any {
        return {
            name: pi.ClipPageId,
            params: {
                clipUuid: this.clip.clip.videoUuid
            },
            query: {
                at: this.accessToken,
            }
        }
    }
}

</script>

<style scoped>

.mini-clip-preview {
    width: 100%;
    height: 200px;
}

.clip-div {
    color: white !important;
}

</style>