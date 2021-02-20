<template>
    <loading-container :is-loading="!allCollectionIds">
        <template v-slot:default="{ loading }">
            <div v-if="!loading">
                <template v-if="allCollectionIds.length > 0">
                    <hearthstone-arena-summary-display
                        v-for="(collection, idx) of allCollectionIds"
                        :key="idx"
                        :arena-uuid="collection"
                        :user-id="userId"
                    >
                    </hearthstone-arena-summary-display>
                </template>

                <div class="text-center" v-else>
                    <span class="text-h5">No Hearthstone Arena runs found.</span>
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
import { Prop, Watch } from 'vue-property-decorator'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { apiClient, HalResponse, ApiData } from '@client/js/api'
import HearthstoneArenaSummaryDisplay from '@client/vue/utility/hearthstone/HearthstoneArenaSummaryDisplay.vue'

const maxTasksPerRequest : number = 20

@Component({
    components: {
        LoadingContainer,
        HearthstoneArenaSummaryDisplay
    }
})
export default class HearthstoneArenaGameLog extends Vue {
    allCollectionIds: string[] | null = null
    lastIndex: number = 0
    nextLink: string | null = null

    @Prop({required: true})
    userId!: number

    get hasNext() : boolean {
        return !!this.nextLink
    }

    @Watch('userId')
    refreshData() {
        this.allCollectionIds = null
        this.nextLink = null
        this.lastIndex = 0
        this.loadMoreData()
    }

    loadMoreData() {
        if (!!this.allCollectionIds && !this.nextLink) {
            return
        }
        apiClient.listHearthstoneArenaRunsForPlayer({
            next: this.nextLink,
            userId: this.userId,
            start: this.lastIndex,
            end: this.lastIndex + maxTasksPerRequest
        }).then((resp : ApiData<HalResponse<string[]>>) => {
            if (!this.allCollectionIds) {
                this.allCollectionIds = resp.data.data
            } else {
                this.allCollectionIds.push(...resp.data.data)
            }
            this.lastIndex += resp.data.data.length
            if ("next" in resp.data._links) {
                this.nextLink = resp.data._links["next"].href
            } else {
                this.nextLink = null
            }
        }).catch((err : any) => {
            console.log('Failed to list Hearthstone arena runs: ' + err);
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>

<style scoped>

</style>