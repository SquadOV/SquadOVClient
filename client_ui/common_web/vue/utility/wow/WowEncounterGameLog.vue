<template>
    <div>
        <wow-filter-ui
            v-model="filters"
            for-raids
            class="my-2"
        >
        </wow-filter-ui>
        <loading-container :is-loading="!allEncounters">
            <template v-slot:default="{ loading }">
                <div v-if="!loading">
                    <wow-encounter-summary
                        class="mb-4"
                        v-for="(encounter, idx) in allEncounters"
                        :key="idx"
                        :encounter="encounter"
                        :user-id="userId"
                    >
                    </wow-encounter-summary>

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
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, HalResponse, ApiData } from '@client/js/api'
import { WowEncounter } from '@client/js/wow/matches'
import { WowMatchFilters, createEmptyWowMatchFilters } from '@client/js/wow/filters'
import WowEncounterSummary from '@client/vue/utility/wow/WowEncounterSummary.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import WowFilterUi from '@client/vue/utility/wow/WowFilterUi.vue'

const maxTasksPerRequest : number = 10

@Component({
    components: {
        LoadingContainer,
        WowEncounterSummary,
        WowFilterUi,
    }
})
export default class WowEncounterGameLog extends Vue {
    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    guid!: string

    allEncounters: WowEncounter[] | null = null
    lastIndex: number = 0
    nextLink: string | null = null
    filters: WowMatchFilters = createEmptyWowMatchFilters()

    get hasNext() : boolean {
        return !!this.nextLink
    }

    @Watch('userId')
    @Watch('guid')
    @Watch('filters', {deep: true})
    refreshData() {
        this.allEncounters = null
        this.lastIndex = 0
        this.nextLink = null
        this.loadMore()
    }

    loadMore() {
        if (!!this.allEncounters && !this.nextLink) {
            return
        }
        apiClient.listWoWEncountersForCharacter({
            next: this.nextLink,
            userId: this.userId,
            guid: this.guid,
            start: this.lastIndex,
            end: this.lastIndex + maxTasksPerRequest,
            filters: this.filters,
        }).then((resp: ApiData<HalResponse<WowEncounter[]>>) => {
            if (!this.allEncounters) {
                this.allEncounters = resp.data.data
            } else {
                this.allEncounters.push(...resp.data.data)
            }
            this.lastIndex += resp.data.data.length
            if ("next" in resp.data._links) {
                this.nextLink = resp.data._links["next"].href
            } else {
                this.nextLink = null
            }
        }).catch((err: any) => {
            console.log('Failed to obtain WoW encounters: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>