<template>
    <div class="py-2">
        <template v-if="!!cards">
            <div v-for="(group, idx) of groupedCards"
                :key="idx"
                v-if="group.cards.length > 0"
                class="px-2"
            >
                <div class="d-flex justify-start">
                    <div class="d-flex align-center mr-4 text--white text-h6 font-weight-bold">
                        {{ group.group }}
                    </div>

                    <div class="d-flex flex-wrap">
                        <div
                            v-for="card of group.cards"
                            :key="`${idx}-${card.cardId}`"
                            class="ma-2"
                        >
                            <hearthstone-mini-card-display
                                :card-id="card.cardId"
                                :max-height="48"
                                history
                                is-friendly
                            >
                            </hearthstone-mini-card-display>
                        </div>
                    </div>
                </div>

                <v-divider></v-divider>
            </div>
        </template>

        <v-row v-else justify="center">
            <v-progress-circular size="64" indeterminate>
            </v-progress-circular>
        </v-row>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { HearthstoneBattlegroundsCardMetadata, HearthstoneDeckSlotWithMetadata } from '@client/js/hearthstone/hearthstone_deck'
import { HearthstoneCardRace, cardRaceToString } from '@client/js/hearthstone/hearthstone_cardtype'
import { apiClient, ApiData } from '@client/js/api'
import HearthstoneMiniCardDisplay from '@client/vue/utility/hearthstone/HearthstoneMiniCardDisplay.vue'

interface GroupedCards {
    group: string
    cards: HearthstoneDeckSlotWithMetadata[]
}

const bgRaceGroups = [
    HearthstoneCardRace.Pet,
    HearthstoneCardRace.Demon,
    HearthstoneCardRace.Dragon,
    HearthstoneCardRace.Elemental,
    HearthstoneCardRace.Mechanical,
    HearthstoneCardRace.Murloc,
    HearthstoneCardRace.Pirate,
    HearthstoneCardRace.Invalid,
]

@Component({
    components: {
        HearthstoneMiniCardDisplay
    }
})
export default class HearthstoneBattlegroundTavernTechLevelSummary extends Vue {
    @Prop({required: true})
    level!: number

    // Pull card metadata and then sort by race before displaying to the user.
    cards: HearthstoneBattlegroundsCardMetadata[] | null = null

    get groupedCards(): GroupedCards[] | null {
        if (!this.cards) {
            return null
        }

        let grouped = new Map<HearthstoneCardRace, HearthstoneBattlegroundsCardMetadata[]>()
        for (let c of this.cards) {
            let race = (c.cardRace !== null) ? c.cardRace : HearthstoneCardRace.Invalid
            if (!grouped.has(race)) {
                grouped.set(race, [])
            }

            let arr = grouped.get(race)
            arr!.push(c)
        }

        return bgRaceGroups.map((g : HearthstoneCardRace ) => {
            let cards = grouped.get(g)
            return {
                group: cardRaceToString(g),
                cards: !!cards ? 
                    cards.map((ele: HearthstoneBattlegroundsCardMetadata) => {
                        return {
                            index: 0,
                            cardId: ele.base.cardId,
                            owned: true,
                            count: {
                                normal: 1,
                                golden: 0,
                            },
                            metadata: ele.base,
                        }
                    })
                    : []
            }
        })
    }

    refreshData() {
        apiClient.accessToken().getHearthstoneBattlegroundsCardsForTavernLevel(this.level).then((resp: ApiData<HearthstoneBattlegroundsCardMetadata[]>) => {
            this.cards = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain Battleground cards for tavern level: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>