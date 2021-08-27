<template>
    <div>
        <hearthstone-filter-ui
            v-model="filters"
            class="my-2"
        >
        </hearthstone-filter-ui>
        
        <v-row>
            <v-col cols="9">
                <loading-container :is-loading="!allMatchIds">
                    <template v-slot:default="{ loading }">
                        <div v-if="!loading">
                            <hearthstone-match-scroller
                                :matches="allMatchIds"
                                v-if="allMatchIds.length > 0"
                                :user-id="userId"
                            >
                            </hearthstone-match-scroller>

                            <div class="text-center" v-else>
                                <span class="text-h5">No Arena games found.</span>
                            </div>
                        </div>
                    </template>
                </loading-container>
            </v-col>

            <v-col cols="3">
                <loading-container :is-loading="!currentArenaRun">
                    <template v-slot:default="{ loading }">
                        <div v-if="!loading">
                            <hearthstone-match-deck-display
                                :deck="currentArenaRun.deck"
                                v-if="!!currentArenaRun.deck"
                            >
                            </hearthstone-match-deck-display>

                            <div class="text-center" v-else>
                                <span class="text-h5">No deck found.</span>
                            </div>
                        </div>
                    </template>
                </loading-container>
            </v-col>
        </v-row>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { HearthstoneArenaRun } from '@client/js/hearthstone/hearthstone_arena'
import { HearthstoneMatchFilters, createEmptyHearthstoneMatchFilters } from '@client/js/hearthstone/filters'
import HearthstoneFilterUi from '@client/vue/utility/hearthstone/HearthstoneFilterUi.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import HearthstoneMatchDeckDisplay from '@client/vue/utility/hearthstone/HearthstoneMatchDeckDisplay.vue'
import HearthstoneMatchScroller from '@client/vue/utility/hearthstone/HearthstoneMatchScroller.vue'

@Component({
    components: {
        LoadingContainer,
        HearthstoneMatchDeckDisplay,
        HearthstoneMatchScroller,
        HearthstoneFilterUi,
    }
})
export default class HearthstoneArenaRunMatchLog extends Vue {
    @Prop({required: true})
    runId!: string

    @Prop({required: true})
    userId!: number

    filters: HearthstoneMatchFilters = createEmptyHearthstoneMatchFilters()

    allMatchIds: string[] | null = null
    currentArenaRun: HearthstoneArenaRun | null = null

    @Watch('userId')
    @Watch('filters', {deep: true})
    refreshData() {
        this.allMatchIds = null
        apiClient.listHearthstoneMatchesForArenaRun(this.runId, this.userId, this.filters).then((resp : ApiData<string[]>) => {
            this.allMatchIds = resp.data
        }).catch((err: any) => {
            console.error('Failed to list Hearthstone arena matches: ' + err);            
        })

        this.currentArenaRun = null
        apiClient.getHearthstoneArenaRun(this.runId, this.userId).then((resp : ApiData<HearthstoneArenaRun>) => {
            this.currentArenaRun = resp.data
        }).catch((err: any) => {
            console.error('Failed to get Hearthstone Arena Run: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}
</script>