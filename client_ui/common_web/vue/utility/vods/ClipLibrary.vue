<template>
    <loading-container :is-loading="!clips">
        <template v-slot:default="{ loading }">
            <v-container fluid v-if="!loading">
                <v-row v-if="clips.length > 0">
                    <v-col cols="3" v-for="(c, idx) in clips" :key="idx">
                        <mini-clip-preview
                            :clip="c"
                        >
                        </mini-clip-preview>
                    </v-col>
                </v-row>

                <div class="pa-4 text-h6 font-weight-bold" v-else>
                    No clips yet!
                </div>

                <v-btn
                    v-if="hasNext"
                    color="primary"
                    block
                    @click="loadMoreData"  
                >
                    Load More
                </v-btn>
            </v-container>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { VodClip } from '@client/js/squadov/vod'
import { apiClient, HalResponse, ApiData } from '@client/js/api'
import MiniClipPreview from '@client/vue/utility/vods/MiniClipPreview.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'

const maxClipsPerRequest: number = 20

@Component({
    components: {
        MiniClipPreview,
        LoadingContainer,
    }
})
export default class ClipLibrary extends Vue {
    @Prop()
    matchUuid!: string | undefined

    clips: VodClip[] | null = null
    lastIndex: number = 0
    nextLink: string | null = null

    get hasNext() : boolean {
        return !!this.nextLink
    }

    @Watch('matchUuid')
    refreshClips() {
        this.clips = null
        this.nextLink = null
        this.lastIndex = 0
        this.loadMoreData()
    }

    loadMoreData() {
        if (!!this.clips && !this.nextLink) {
            return
        }

        apiClient.listClips({
            next: this.nextLink,
            matchUuid: this.matchUuid,
            start: this.lastIndex,
            end: this.lastIndex + maxClipsPerRequest
        }).then((resp : ApiData<HalResponse<VodClip[]>>) => {
            if (!this.clips) {
                this.clips = resp.data.data
            } else {
                this.clips.push(...resp.data.data)
            }
            this.lastIndex += resp.data.data.length
            if ("next" in resp.data._links) {
                this.nextLink = resp.data._links["next"].href
            } else {
                this.nextLink = null
            }
        }).catch((err : any) => {
            console.log('Failed to list clips: ' + err);
        })
    }

    mounted() {
        this.refreshClips()
    }
}

</script>