<template>
    <loading-container :is-loading="!allChallenges">
        <template v-slot:default="{ loading }">
            <div v-if="!loading">
                <wow-keystone-summary
                    class="mb-4"
                    v-for="(challenge, idx) in allChallenges"
                    :key="idx"
                    :challenge="challenge"
                    :user-id="userId"
                >
                </wow-keystone-summary>

                <v-btn
                    v-if="hasNext"
                    color="primary"
                    block
                    @click="loadMore"  
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
import { apiClient, HalResponse, ApiData } from '@client/js/api'
import { WowChallenge } from '@client/js/wow/matches'
import WowKeystoneSummary from '@client/vue/utility/wow/WowKeystoneSummary.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'

const maxTasksPerRequest : number = 20

@Component({
    components: {
        LoadingContainer,
        WowKeystoneSummary
    }
})
export default class WowKeystoneGameLog extends Vue {
    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    guid!: string

    allChallenges: WowChallenge[] | null = null
    lastIndex: number = 0
    nextLink: string | null = null

    get hasNext() : boolean {
        return !!this.nextLink
    }

    @Watch('userId')
    @Watch('guid')
    refreshData() {
        this.allChallenges = null
        this.lastIndex = 0
        this.nextLink = null
        this.loadMore()
    }

    loadMore() {
        if (!!this.allChallenges && !this.nextLink) {
            return
        }
        apiClient.listWoWChallengesForCharacter({
            next: this.nextLink,
            userId: this.userId,
            guid: this.guid,
            start: this.lastIndex,
            end: this.lastIndex + maxTasksPerRequest,
        }).then((resp: ApiData<HalResponse<WowChallenge[]>>) => {
            if (!this.allChallenges) {
                this.allChallenges = resp.data.data
            } else {
                this.allChallenges.push(...resp.data.data)
            }
            this.lastIndex += resp.data.data.length
            if ("next" in resp.data._links) {
                this.nextLink = resp.data._links["next"].href
            } else {
                this.nextLink = null
            }
        }).catch((err: any) => {
            console.log('Failed to obtain WoW challenges: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>