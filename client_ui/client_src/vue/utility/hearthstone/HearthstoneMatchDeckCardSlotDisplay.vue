<template>
    <v-tooltip
        right
        color="transparent"
    >
        <template v-slot:activator="{on, attrs}">
            <div class="d-flex full-width slot-div" v-bind="attrs" v-on="on">
                <!-- Card cost/rarity display -->
                <div class="rarity-div d-flex justify-center align-center flex-shrink-0 flex-grow-0" :style="rarityStyle">
                    <div>
                        {{ cardSlot.metadata.cost }}
                    </div>
                </div>

                <!-- Card Display (name + background) -->
                <div class="d-flex align-center flex-grow-1 flex-shrink-1" :style="nameStyle">
                    <div class="ml-4">
                        {{ cardSlot.metadata.name }}
                    </div>
                </div>

                <!-- Count count display (if > 1 or legendary) -->
                <div v-if="totalCount > 1 || isLegendary" class="count-div d-flex justify-center align-center flex-shrink-0 flex-grow-0">
                    <div>
                        <span v-if="totalCount > 1">
                            {{ totalCount }}
                        </span>

                        <span v-else>
                            <v-icon small color="#FFC107">
                                mdi-star
                            </v-icon>
                        </span>
                    </div>
                </div>
            </div>
        </template>
        <hearthstone-full-card-display
            :card-id="cardSlot.cardId"
        >
        </hearthstone-full-card-display>
    </v-tooltip>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import {  HearthstoneDeckSlotWithMetadata, HearthstoneCardRarity } from '@client/js/hearthstone/hearthstone_deck'
import { getRarityColor } from '@client/js/hearthstone/hearthstone_colors'
import { staticClient } from '@client/js/staticData'
import HearthstoneFullCardDisplay from '@client/vue/utility/hearthstone/HearthstoneFullCardDisplay.vue'

@Component({
    components: {
        HearthstoneFullCardDisplay
    }
})
export default class HearthstoneMatchDeckCardSlotDisplay extends Vue {
    @Prop({required: true})
    cardSlot!: HearthstoneDeckSlotWithMetadata

    get totalCount(): number {
        return this.cardSlot.count.normal + this.cardSlot.count.golden
    }

    get isLegendary(): boolean {
        return this.cardSlot.metadata.rarity == HearthstoneCardRarity.Legendary
    }

    get rarityStyle() : any {
        let color = getRarityColor(this.cardSlot.metadata.rarity)
        return {
            'background-color': `rgb(${color.r}, ${color.g}, ${color.b})`
        }
    }

    get cardSrc() : string {
        return staticClient.getHearthstoneCardPortraitUrl(this.cardSlot.cardId)
    }

    get nameStyle(): any {
        return {
            'font-weight': 700,
            'background-image': `linear-gradient(to right, #121212 0 10%, transparent), url('${this.cardSrc}')`,
            'background-position': 'right -64px top -128px',
            'background-size': `256px 256px`
        }
    }
}

</script>

<style scoped>

.slot-div {
    border: 1px solid white;
}

.rarity-div {
    width: 32px;
    height: 32px;
    flex: 0 0 auto;
    border: 1px solid gray;
    font-weight: 700;
}

.count-div {
    width: 32px;
    height: 32px;
    flex: 0 0 auto;
    border: 1px solid gray;
    font-weight: 700;
    background-color: #343434;
    color: #FFC107;
}

</style>