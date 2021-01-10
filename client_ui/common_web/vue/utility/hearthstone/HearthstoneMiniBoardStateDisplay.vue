<template>
    <div class="d-flex justify-space-around">
        <hearthstone-mini-card-display
            v-for="(cardId, idx) in orderedCardIds"
            :card-id="cardId"
            :key="idx"
        >
        </hearthstone-mini-card-display>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { HearthstoneMatchSnapshot } from '@client/js/hearthstone/hearthstone_snapshot'
import { HearthstoneEntity, HearthstoneEntityWrapper } from '@client/js/hearthstone/hearthstone_entity'
import { HearthstoneCardtype } from '@client/js/hearthstone/hearthstone_cardtype'
import { HearthstoneZone } from '@client/js/hearthstone/hearthstone_zone'
import HearthstoneMiniCardDisplay from '@client/vue/utility/hearthstone/HearthstoneMiniCardDisplay.vue'

@Component({
    components: {
        HearthstoneMiniCardDisplay
    }
})
export default class HearthstoneMiniBoardStateDisplay extends Vue {
    @Prop({required: true})
    snapshot!: HearthstoneMatchSnapshot | null

    @Prop({required: true})
    playerMatchId!: number | undefined

    get orderedCardIds() : string[] {
        if (!this.snapshot || !this.playerMatchId) {
            return []
        }

        let relevantEntities: HearthstoneEntityWrapper[]  = []
        for (let [eid, entity] of Object.entries(this.snapshot.entities)) {
            let typedEntity: HearthstoneEntity = entity
            let wrappedEntity = new HearthstoneEntityWrapper(typedEntity)
            if (wrappedEntity.controller == this.playerMatchId 
                && wrappedEntity.zone == HearthstoneZone.Play
                && wrappedEntity.cardType == HearthstoneCardtype.Minion) {
                relevantEntities.push(wrappedEntity)
            }
        }

        relevantEntities.sort((a: HearthstoneEntityWrapper, b : HearthstoneEntityWrapper) => {
            return a.zonePosition - b.zonePosition
        })

        return relevantEntities.map((ele: HearthstoneEntityWrapper) => ele.cardId)
    }
}

</script>