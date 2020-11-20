<template>
    <loading-container :is-loading="!allCollectionIds">
        <template v-slot:default="{ loading }">
            <div v-if="!loading">
                <div class="text-center">
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
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { apiClient, HalResponse, ApiData } from '@client/js/api'

const maxTasksPerRequest : number = 20

@Component({
    components: {
        LoadingContainer,

    }
})
export default class HearthstoneDuelGameLog extends Vue {
    allCollectionIds: string[] | null = null
    lastIndex: number = 0
    nextLink: string | null = null

    get hasNext() : boolean {
        return !!this.nextLink
    }

    loadMoreData() {
        if (!!this.allCollectionIds && !this.nextLink) {
            return
        }
        let user = this.$store.state.currentUser!
        apiClient.listHearthstoneMatchesForPlayer({
            next: this.nextLink,
            userId: user.id,
            start: this.lastIndex,
            end: this.lastIndex + maxTasksPerRequest,
            filter: [],
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
        this.loadMoreData()
    }
}

</script>

<style scoped>

</style>