<template>
    <div>
        <router-link :to="to">
            <v-sheet
                class="task-summary"
                rounded
            >
                <v-row no-gutters v-if="!!currentArenaRun">
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
                            {{ currentArenaRun.wins }}
                        </div>

                        <div class="d-flex justify-center">
                            Wins
                        </div>
                    </v-col>

                    <v-col align-self="center" cols="1">
                        <div class="d-flex justify-center text-h2 red--text">
                            {{ currentArenaRun.loss }}
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
import { HearthstoneArenaRun } from '@client/js/hearthstone/hearthstone_arena'
import { apiClient, ApiData } from '@client/js/api'
import { standardFormatTime } from '@client/js/time'
import HearthstoneHeroDisplay from '@client/vue/utility/hearthstone/HearthstoneHeroDisplay.vue'
import * as pi from '@client/js/pages'

@Component({
    components: {
        HearthstoneHeroDisplay
    }
})
export default class HearthstoneArenaSummaryDisplay extends Vue {
    @Prop({required: true})
    arenaUuid!: string

    @Prop({required: true})
    userId!: number

    currentArenaRun: HearthstoneArenaRun | null = null

    get to() : any {
        return {
            name: pi.HearthstoneArenaRunMatchLogPageId,
            params: {
                arenaId: this.arenaUuid
            }
        }
    }

    refreshData() {
        apiClient.getHearthstoneArenaRun(this.arenaUuid, this.userId).then((resp : ApiData<HearthstoneArenaRun>) => {
            this.currentArenaRun = resp.data
        }).catch((err: any) => {
            console.error('Failed to get Hearthstone Arena Run: ', err)
        })
    }

    get deckHeroCard() : string | undefined {
        return this.currentArenaRun!.deck?.heroCard
    }

    get timestamp() : string {
        return standardFormatTime(this.currentArenaRun!.timestamp)
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