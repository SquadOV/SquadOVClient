<template>
    <div>
        <router-link :to="to">
            <v-sheet
                class="task-summary"
                rounded
            >
                <v-row no-gutters v-if="!!currentDuelRun">
                    <v-col cols="2" align-self="center">
                        <hearthstone-hero-display
                            :hero-card="deckHeroCard"
                            :max-height="150"
                        >
                        </hearthstone-hero-display>
                    </v-col>

                    <v-col align-self="center" cols="3">
                        <div>
                            {{ timestamp }}
                        </div>
                    </v-col>

                    <v-col cols="5">
                    </v-col>

                    <v-col align-self="center" cols="1">
                        <div class="d-flex justify-center text-h2 green--text">
                            {{ currentDuelRun.wins }}
                        </div>

                        <div class="d-flex justify-center">
                            Wins
                        </div>
                    </v-col>

                    <v-col align-self="center" cols="1">
                        <div class="d-flex justify-center text-h2 red--text">
                            {{ currentDuelRun.loss }}
                        </div>

                        <div class="d-flex justify-center">
                            Losses
                        </div>
                    </v-col>
                </v-row>
                <v-row justify="center" v-else>
                    <v-progress-circular indeterminate size=64></v-progress-circular>
                </v-row>
            </v-sheet>
        </router-link>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { HearthstoneDuelRun } from '@client/js/hearthstone/hearthstone_duel'
import { apiClient, ApiData } from '@client/js/api'
import { standardFormatTime } from '@client/js/time'
import HearthstoneHeroDisplay from '@client/vue/utility/hearthstone/HearthstoneHeroDisplay.vue'
import * as pi from '@client/js/pages'

@Component({
    components: {
        HearthstoneHeroDisplay
    }
})
export default class HearthstoneDuelSummaryDisplay extends Vue {
    @Prop({required: true})
    duelUuid!: string

    currentDuelRun: HearthstoneDuelRun | null = null

    get to() : any {
        return {
            name: pi.HearthstoneDuelRunMatchLogPageId,
            params: {
                duelId: this.duelUuid
            }
        }
    }

    refreshData() {
        apiClient.getHearthstoneDuelRun(this.duelUuid, this.$store.state.currentUser!.id).then((resp : ApiData<HearthstoneDuelRun>) => {
            this.currentDuelRun = resp.data
        }).catch((err: any) => {
            console.log('Failed to get Hearthstone Duel Run: ', err)
        })
    }

    get deckHeroCard() : string | undefined {
        return this.currentDuelRun!.heroCard || this.currentDuelRun!.deck?.heroCard
    }

    get timestamp() : string {
        return standardFormatTime(this.currentDuelRun!.timestamp)
    }

    mounted() {
        this.refreshData()
    }
}

</script>

<style scoped>

.task-summary {
    width: 100%;
    position: relative;
}

</style>