<template>
    <div v-if="playerId !== undefined">
        <hearthstone-match-deck-card-slot-display
            v-for="(slot, idx) in statefulSlots"
            :key="idx"
            :card-slot="slot"
            :hide-count="zoneToIcon(slot.zone) == ''"
            :custom-count-icon="zoneToIcon(slot.zone)"
            :opacity="zoneToOpacity(slot.zone)"
        >
        </hearthstone-match-deck-card-slot-display>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { HearthstoneDeck, HearthstoneDeckZoneCardData, HearthstoneDeckSlotWithMetadata } from '@client/js/hearthstone/hearthstone_deck'
import { HearthstoneMatchWrapper } from '@client/js/hearthstone/hearthstone_match'
import { HearthstoneEntityWrapper } from '@client/js/hearthstone/hearthstone_entity'
import { HearthstoneZone } from '@client/js/hearthstone/hearthstone_zone'
import HearthstoneMatchDeckCardSlotDisplay from '@client/vue/utility/hearthstone/HearthstoneMatchDeckCardSlotDisplay.vue'
import HearthstoneMatchDeckDisplay from '@client/vue/utility/hearthstone/HearthstoneMatchDeckDisplay.vue'

@Component
export default class HearthstoneCurrentDeckState extends HearthstoneMatchDeckDisplay {
    @Prop({required: true})
    currentMatch!: HearthstoneMatchWrapper

    @Prop({required: true})
    turn!: number

    @Prop({required: true})
    playerId!: number | undefined

    zoneToOpacity(z: HearthstoneZone) : number {
        switch (z) {
            case HearthstoneZone.Deck:
                return 1.0
            default:
                return 0.3
        }
        return 0.0
    }

    zoneToIcon(z: HearthstoneZone) : string {
        switch (z) {
            case HearthstoneZone.Play:
                return 'mdi-sword-cross'
            case HearthstoneZone.Hand:
                return 'mdi-cards'
            case HearthstoneZone.Graveyard:
                return 'mdi-grave-stone'
            case HearthstoneZone.RemovedFromGame:
                return 'mdi-close'
            case HearthstoneZone.Secret:
                return 'mdi-help'
        }
        return ''
    }
    
    get statefulSlots(): HearthstoneDeckZoneCardData[] {
        let snapshot = this.currentMatch.snapshotForTurn(this.turn)
        if (!snapshot) {
            return this.slots.map((ele: HearthstoneDeckSlotWithMetadata) => ({
                zone: HearthstoneZone.Deck,
                ...ele
            }))
        }

        // For each card in the deck we need to find the corresponding
        // entity in the current match snapshot from the entities that
        // were created at the beginning of the match. If we can't find
        // a correspondence then we can safely assume that the card is still
        // in the player's deck (i.e. it hasn't yet been shown).
        let cardZoneData: HearthstoneDeckZoneCardData[] = []

        // deckEntities is just what we use to grab what entities should be considered
        // a part of the user's "deck".
        let deckEntities = this.currentMatch.deckEntitiesForPlayer(this.playerId!)
        // currentDeckEntities are the entities (and its state) in the current snapshot.
        let currentDeckEntities = deckEntities.map((ele: HearthstoneEntityWrapper) => {
            return snapshot!.entity(ele._entity.entityId)
        })
        
        let cardIdToEntity = new Map<string, HearthstoneEntityWrapper[]>()
        for (let entity of currentDeckEntities) {
            if (!entity || entity.cardId == '') {
                continue
            }

            if (!cardIdToEntity.has(entity.cardId)) {
                cardIdToEntity.set(entity.cardId, [])
            }

            let arr = cardIdToEntity.get(entity.cardId)
            arr!.push(entity)
        }

        for (let slot of this.slots) {
            // Note that an original deck slot could be any number of cards (> 0).
            // We need to break the slot down into individual cards so that we can
            // show the state. Note that we can group cards if they have the same
            // zone for display compactness.
            let total = slot.count.normal + slot.count.golden
            if (total == 0) {
                // This probably shouldn't ever happen but handle just in case?
                continue
            }

            let entities = cardIdToEntity.get(slot.cardId)
            if (!entities) {
                entities = []
            }

            let zoneCount = new Map<HearthstoneZone, number>()
            zoneCount.set(HearthstoneZone.Deck, 0)

            for (let e of entities) {
                // Not really sure when Invalid would crop up but stick it in the deck just to be safe.
                let zone = (e.zone == HearthstoneZone.Invalid) ? HearthstoneZone.Deck : e.zone
                if (!zoneCount.has(zone)) {
                    zoneCount.set(zone, 0)
                }
                zoneCount.set(zone, zoneCount.get(zone)! + 1)
            }

            // Any cards that aren't in the entities list we can safely assume
            // to be in the deck zone.
            zoneCount.set(HearthstoneZone.Deck, zoneCount.get(HearthstoneZone.Deck)! + total - entities.length)

            for (const [key, value] of zoneCount.entries()) {
                if (value == 0) {
                    continue
                }

                cardZoneData.push({
                    zone: key,
                    ...slot
                })
            }
        }

        return cardZoneData
    }
}

</script>