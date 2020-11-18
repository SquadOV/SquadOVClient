<template>
    <v-container fluid v-if="!!subject">
        <v-row no-gutters>
            <v-col align-self="center" cols="1">
                <!-- Block Type indicator (if any) -->
                <v-img
                    v-if="hasTypeIndicator"
                    :max-height="32"
                    :src="typeIndicatorImage"
                    contain
                >
                </v-img>
            </v-col>

            <v-col align-self="center"  cols="11">
                <div class="d-flex justify-around align-center ml-4">
                    <!-- Block Subject -->
                    <hearthstone-mini-card-display
                        :max-height="64"
                        :card-id="subject.cardId"
                        left
                        no-shrink
                        no-overflow
                        history
                        :is-friendly="subject.controller == currentPlayerId"
                    >
                    </hearthstone-mini-card-display>

                    <template v-if="receivers.length > 0">
                        <!-- Image to show that we're doing something to another card -->
                        <v-img
                            :max-height="32"
                            :src="cardActionImage"
                            contain
                            class="flex-shrink-1 flex-grow-0"
                        >
                        </v-img>

                        <!-- Block Receiver (if any) -->
                        <hearthstone-mini-card-display
                            v-for="(recv, idx) of receivers"
                            :max-height="64"
                            :key="idx"
                            :card-id="recv.cardId"
                            left
                            no-shrink
                            no-overflow
                            history
                            :is-friendly="recv.controller == currentPlayerId"
                        >
                        </hearthstone-mini-card-display>
                    </template>
                </div>
            </v-col>
        </v-row>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { HearthstoneMatchWrapper } from '@client/js/hearthstone/hearthstone_match'
import { HearthstoneGameBlockWrapper, BlockType } from '@client/js/hearthstone/hearthstone_actions'
import { HearthstoneEntityWrapper } from '@client/js/hearthstone/hearthstone_entity'
import HearthstoneMiniCardDisplay from '@client/vue/utility/hearthstone/HearthstoneMiniCardDisplay.vue'

@Component({
    components: {
        HearthstoneMiniCardDisplay
    }
})
export default class HearthstoneGameBlockRenderer extends Vue {
    @Prop({required: true})
    currentMatch!: HearthstoneMatchWrapper

    @Prop({required: true})
    block!: HearthstoneGameBlockWrapper

    get hasTypeIndicator(): boolean {
        return this.typeIndicatorImage.length != 0
    }

    get currentPlayerId(): number | undefined {
        return this.currentMatch.currentPlayerId
    }

    get subject(): HearthstoneEntityWrapper | undefined {
        return this.block.subject
    }

    get receivers(): HearthstoneEntityWrapper[] {
        return this.block.targets
    }

    get typeIndicatorImage(): string {
        if (this.block.blockType == BlockType.Attack) {
            // An attack is always an attack so display the sword image.
            return 'assets/hearthstone/Attack.png'
        } else if (this.block.blockType == BlockType.Play) {
            // Playing a card is a little trickier as we could play a minion
            // or play a spell. In the case where we play a spell we display
            // the spell image if the spell does something to the *opposing* player
            // (what we can an "offensive" action).
            if (this.block.isSpell && this.block.isOffensive) {
                return 'assets/hearthstone/Spell.png'
            }
        }
        return ''
    }

    get cardActionImage(): string {
        if (this.block.isOffensive) {
            return 'assets/hearthstone/RedArrow.png'
        } else {
            return 'assets/hearthstone/BlueArrow.png'
        }
    }
}

</script>