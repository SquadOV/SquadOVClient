<template>
    <v-card>
        <v-card-title>
            Clips
        </v-card-title>
        <v-divider></v-divider>

        <loading-container :is-loading="!clipPages">
            <template v-slot:default="{ loading }">
                <v-carousel
                    @change="onChangePageIdx"
                    show-arrows
                    show-arrows-on-hover
                    hide-delimiter-background
                    hide-delimiters
                    height="auto"
                    v-if="!loading"
                >
                    <v-carousel-item
                        v-for="(page, idx) in clipPages"
                        :key="`page-${idx}`"
                    >
                        <loading-container :is-loading="!page">
                            <template v-slot:default="{ loading : pageLoading }">
                                <v-container fluid v-if="!pageLoading">
                                    <v-row no-gutters>
                                        <v-col cols="4"
                                            v-for="clip in page"
                                            :key="clip.clip.videoUuid"
                                        >
                                            <mini-clip-preview
                                                class="flex-grow-1 ma-1"
                                                :access-token="clip.accessToken"
                                                :clip="clip"
                                                hide-details
                                                hide-tags
                                            >
                                            </mini-clip-preview>
                                        </v-col>
                                    </v-row>
                                </v-container>
                            </template>
                        </loading-container>
                    </v-carousel-item>
                </v-carousel>
            </template>
        </loading-container>
    </v-card>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { VodClip } from '@client/js/squadov/vod'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { apiClient, ApiData, HalResponse } from '@client/js/api'
import { RecentMatchFilters, createEmptyRecentMatchFilters } from '@client/js/squadov/recentMatch'
import MiniClipPreview from '@client/vue/utility/vods/MiniClipPreview.vue'

const maxClipsPerRequest: number = 12

@Component({
    components: {
        LoadingContainer,
        MiniClipPreview,
    }
})
export default class UserProfileClips extends Vue {
    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    accessToken!: string

    clipPages: (VodClip[] | null)[] | null = null
    lastIndex: number = 0
    nextLink: string | null = null

    onChangePageIdx(idx: number) {
        if (!this.clipPages) {
            return
        }

        if (idx < 0 || idx >= this.clipPages.length) {
            return
        }

        if (!this.clipPages[idx]) {
            this.loadMoreData()
        }
    }

    @Watch('userId')
    refreshData() {
        this.clipPages = null
        this.lastIndex = 0
        this.nextLink = null
        this.loadMoreData()
    }

    get filters(): RecentMatchFilters {
        let f = createEmptyRecentMatchFilters()
        f.users = [this.userId]
        return f
    }

    loadMoreData() {
        if (!!this.clipPages && !this.nextLink) {
            return
        }

        apiClient.accessToken(this.accessToken).listClips({
            next: this.nextLink,
            matchUuid: undefined,
            start: this.lastIndex,
            end: this.lastIndex + maxClipsPerRequest,
            filters: this.filters,
            profileId: this.userId
        }).then((resp : ApiData<HalResponse<VodClip[]>>) => {
            if (!this.clipPages) {
                this.clipPages = [resp.data.data]
            } else {
                Vue.set(this.clipPages, this.clipPages.length - 1, resp.data.data)
            }

            this.lastIndex += resp.data.data.length
            if ("next" in resp.data._links) {
                this.nextLink = resp.data._links["next"].href
                this.clipPages.push(null)
            } else {
                this.nextLink = null
            }
        }).catch((err : any) => {
            console.error('Failed to list clips: ' + err);
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>