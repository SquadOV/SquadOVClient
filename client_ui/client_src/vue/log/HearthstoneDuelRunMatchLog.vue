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
                            <span class="text-h5">No Duel games found.</span>
                        </div>
                    </div>
                </template>
            </loading-container>
        </v-col>

        <v-col cols="3">
            <loading-container :is-loading="!currentDuelRun">
                <template v-slot:default="{ loading }">
                    <div v-if="!loading">
                        <hearthstone-match-deck-display
                            :deck="currentDuelRun.deck"
                            v-if="!!currentDuelRun.deck"
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
import { HearthstoneDuelRun } from '@client/js/hearthstone/hearthstone_duel'
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
export default class HearthstoneDuelRunMatchLog extends Vue {
    @Prop({required: true})
    runId!: string

    allMatchIds: string[] | null = null
    currentDuelRun: HearthstoneDuelRun | null = null

    refreshData() {
        apiClient.listHearthstoneMatchesForDuelRun(this.runId, this.$store.state.currentUser!.id).then((resp : ApiData<string[]>) => {
            this.allMatchIds = resp.data
        }).catch((err: any) => {
            console.log('Failed to list Hearthstone duel matches: ' + err);            
        })

        apiClient.getHearthstoneDuelRun(this.runId, this.$store.state.currentUser!.id).then((resp : ApiData<HearthstoneDuelRun>) => {
            this.currentDuelRun = resp.data
        }).catch((err: any) => {
            console.log('Failed to get Hearthstone Duel Run: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}
</script>