<template>
    <div>
        <hearthstone-match-deck-card-slot-display
            v-for="(slot, idx) in slots"
            :key="idx"
            :card-slot="slot"
        >
        </hearthstone-match-deck-card-slot-display>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { HearthstoneDeck, HearthstoneDeckSlotWithMetadata, HearthstoneCardMetadata, HearthstoneDeckSlot } from '@client/js/hearthstone/hearthstone_deck'
import HearthstoneMatchDeckCardSlotDisplay from '@client/vue/utility/hearthstone/HearthstoneMatchDeckCardSlotDisplay.vue'
import { apiClient, ApiData } from '@client/js/api'

@Component({
    components: {
        HearthstoneMatchDeckCardSlotDisplay
    }
})
export default class HearthstoneMatchDeckDisplay extends Vue {
    @Prop({required: true})
    deck!: HearthstoneDeck

    slots: HearthstoneDeckSlotWithMetadata[] = []

    // We need to retrieve metadata about all the cards in the deck
    // as it'll contain information about what the card's name is and its
    // cost/rarity so we can render the slot properly.
    refreshData() {
        this.slots = []

        let originalSlots: Map<string, HearthstoneDeckSlot> = new Map()
        for (let s of this.deck.slots) {
            originalSlots.set(s.cardId, s)
        }

        apiClient.getBulkHearthstoneCardMetadata(this.deck.slots.map((slot: HearthstoneDeckSlot) => {
            return slot.cardId
        })).then((resp : ApiData<HearthstoneCardMetadata[]>) => {
            this.slots = resp.data.map((ele: HearthstoneCardMetadata) => {
                return {
                    metadata: ele,
                    ...originalSlots.get(ele.cardId)!
                }
            })

            this.slots.sort((a: HearthstoneDeckSlotWithMetadata, b: HearthstoneDeckSlotWithMetadata) => {
                return a.metadata.cost - b.metadata.cost
            })
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>