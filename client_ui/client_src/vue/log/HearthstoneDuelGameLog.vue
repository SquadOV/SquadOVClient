<template>
    <loading-container :is-loading="!allCollectionIds">
        <template v-slot:default="{ loading }">
            <div v-if="!loading">
                <template v-if="allCollectionIds.length > 0">
                    <hearthstone-duel-summary-display
                        v-for="(collection, idx) of allCollectionIds"
                        :key="idx"
                        :duel-uuid="collection"
                        :user-id="userId"
                    >
                    </hearthstone-duel-summary-display>
                </template>

                <div class="text-center" v-else>
                    <span class="text-h5">No Hearthstone Duel runs found.</span>
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
import HearthstoneDuelSummaryDisplay from '@client/vue/utility/hearthstone/HearthstoneDuelSummaryDisplay.vue'

const maxTasksPerRequest : number = 20

@Component({
    components: {
        LoadingContainer,
        HearthstoneDuelSummaryDisplay
    }
})
export default class HearthstoneDuelGameLog extends Vue {
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
        this.lastIndex = 0
        this.nextLink = null
        this.allCollectionIds = null
        this.loadMoreData()
    }

    loadMoreData() {
        if (!!this.allCollectionIds && !this.nextLink) {
            return
        }
        
        apiClient.listHearthstoneDuelRunsForPlayer({
            next: this.nextLink,
            userId: this.userId,
            start: this.lastIndex,
            end: this.lastIndex + maxTasksPerRequest,
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
            console.log('Failed to list Hearthstone duel runs: ' + err);
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>

<style scoped>

</style>