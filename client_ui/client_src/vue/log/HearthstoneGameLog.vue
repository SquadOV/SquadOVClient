<template>
    <loading-container :is-loading="!allMatchIds">
        <template v-slot:default="{ loading }">
            <div v-if="!loading">
                <hearthstone-match-scroller
                    :matches="allMatchIds"
                    v-if="allMatchIds.length > 0"
                >
                </hearthstone-match-scroller>

                <div class="text-center" v-else>
                    <span class="text-h5">No Hearthstone games found.</span>
                </div>

                <v-btn
                    v-if="hasNext"
                    color="primary"
                    block
                    @click="loadMoreData"  
                >
                    Load More
                </v-btn>
            </div>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { apiClient, HalResponse, ApiData } from '@client/js/api'
import HearthstoneMatchScroller from '@client/vue/utility/hearthstone/HearthstoneMatchScroller.vue'

const maxTasksPerRequest : number = 20

@Component({
    components: {
        LoadingContainer,
        HearthstoneMatchScroller
    }
})
export default class HearthstoneGameLog extends Vue {
    allMatchIds: string[] | null = null
    lastIndex: number = 0
    nextLink: string | null = null

    get hasNext() : boolean {
        return !!this.nextLink
    }

    loadMoreData() {
        if (!!this.allMatchIds && !this.nextLink) {
            return
        }
        let user = this.$store.state.currentUser!
        apiClient.listHearthstoneMatchesForPlayer({
            next: this.nextLink,
            userId: user.id,
            start: this.lastIndex,
            end: this.lastIndex + maxTasksPerRequest
        }).then((resp : ApiData<HalResponse<string[]>>) => {
            if (!this.allMatchIds) {
                this.allMatchIds = resp.data.data
            } else {
                this.allMatchIds.push(...resp.data.data)
            }
            this.lastIndex += resp.data.data.length
            if ("next" in resp.data._links) {
                this.nextLink = resp.data._links["next"].href
            } else {
                this.nextLink = null
            }
        }).catch((err : any) => {
            console.log('Failed to list Hearthstone matches: ' + err);
        })
    }

    mounted() {
        this.loadMoreData()
    }
}

</script>

<style scoped>

</style>