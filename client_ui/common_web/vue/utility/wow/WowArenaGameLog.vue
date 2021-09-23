<template>
    <div>
        <wow-filter-ui
            v-model="filters"
            for-arenas
            class="my-2"
            :release="release"
        >
        </wow-filter-ui>
        <loading-container :is-loading="!allArenas">
            <template v-slot:default="{ loading }">
                <div v-if="!loading">
                    <wow-arena-summary
                        class="mb-4"
                        v-for="(arena, idx) in allArenas"
                        :key="idx"
                        :arena="arena"
                        :user-id="userId"
                    >
                    </wow-arena-summary>

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
import { WowArena } from '@client/js/wow/matches'
import { WowMatchFilters, createEmptyWowMatchFilters } from '@client/js/wow/filters'
import WowArenaSummary from '@client/vue/utility/wow/WowArenaSummary.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import WowFilterUi from '@client/vue/utility/wow/WowFilterUi.vue'
import { WowGameRelease } from '@client/js/staticData'

const maxTasksPerRequest : number = 10

@Component({
    components: {
        LoadingContainer,
        WowArenaSummary,
        WowFilterUi,
    }
})
export default class WowArenaGameLog extends Vue {
    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    guid!: string
    
    @Prop({required: true})
    release!: WowGameRelease

    allArenas: WowArena[] | null = null
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
        this.allArenas = null
        this.lastIndex = 0
        this.nextLink = null
        this.loadMore()
    }

    loadMore() {
        if (!!this.allArenas && !this.nextLink) {
            return
        }
        apiClient.listWoWArenasForCharacter({
            next: this.nextLink,
            userId: this.userId,
            guid: this.guid,
            start: this.lastIndex,
            end: this.lastIndex + maxTasksPerRequest,
            filters: this.filters,
        }).then((resp: ApiData<HalResponse<WowArena[]>>) => {
            if (!this.allArenas) {
                this.allArenas = resp.data.data
            } else {
                this.allArenas.push(...resp.data.data)
            }
            this.lastIndex += resp.data.data.length
            if ("next" in resp.data._links) {
                this.nextLink = resp.data._links["next"].href
            } else {
                this.nextLink = null
            }
        }).catch((err: any) => {
            console.error('Failed to obtain WoW arenas: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>