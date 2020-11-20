<template>
    <v-row>
        <v-col cols="9">
            <loading-container :is-loading="!allMatchIds">
                <template v-slot:default="{ loading }">
                    <div v-if="!loading">
                        <hearthstone-match-scroller
                            :matches="allMatchIds"
                            v-if="allMatchIds.length > 0"
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
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { HearthstoneArenaRun } from '@client/js/hearthstone/hearthstone_arena'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import HearthstoneMatchDeckDisplay from '@client/vue/utility/hearthstone/HearthstoneMatchDeckDisplay.vue'
import HearthstoneMatchScroller from '@client/vue/utility/hearthstone/HearthstoneMatchScroller.vue'

@Component({
    components: {
        LoadingContainer,
        HearthstoneMatchDeckDisplay,
        HearthstoneMatchScroller
    }
})
export default class HearthstoneArenaRunMatchLog extends Vue {
    @Prop({required: true})
    runId!: string

    allMatchIds: string[] | null = null
    currentArenaRun: HearthstoneArenaRun | null = null

    refreshData() {
        apiClient.listHearthstoneMatchesForArenaRun(this.runId, this.$store.state.currentUser!.id).then((resp : ApiData<string[]>) => {
            this.allMatchIds = resp.data
        }).catch((err: any) => {
            console.log('Failed to list Hearthstone arena matches: ' + err);            
        })

        apiClient.getHearthstoneArenaRun(this.runId, this.$store.state.currentUser!.id).then((resp : ApiData<HearthstoneArenaRun>) => {
            this.currentArenaRun = resp.data
        }).catch((err: any) => {
            console.log('Failed to get Hearthstone Arena Run: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}
</script>