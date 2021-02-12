<template>
    <v-sheet class="full-parent-height">
        <div class="d-flex align-center pa-4">
            <span class="text-h5">
                {{ roundName }}
            </span>

            <v-spacer></v-spacer>
            
            <v-btn
                class="ml-4"
                @click="goToTurn"
                color="primary"
                fab
                small
                v-if="!!turnTime && hasVod"
            >
                <v-icon>mdi-play</v-icon>
            </v-btn>
        </div>
        <v-divider></v-divider>

        <v-list class="event-list pa-0">
            <v-list-item
                v-for="(block, index) in relevantBlocks"
                :key="`${block.blockId}-${index}`"
                :style="eventStyling(block)"
            >
                <v-list-item-content class="event-time">
                    {{ blockTimeDisplay(block) }}
                </v-list-item-content>

                <v-list-item-content class="px-4">
                    <hearthstone-game-block-renderer
                        :current-match="currentMatch"
                        :block="block"
                    >
                    </hearthstone-game-block-renderer>
                </v-list-item-content>

                <v-list-item-action class="ml-0" v-if="hasVod && !!block.blockTime">
                    <v-btn
                        outlined
                        fab
                        small
                        color="white"
                        @click="goToEvent(block)"
                    >
                        <v-icon>mdi-play</v-icon>
                    </v-btn>
                </v-list-item-action>
            </v-list-item>
        </v-list>
    </v-sheet>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { HearthstoneMatchWrapper } from '@client/js/hearthstone/hearthstone_match'
import { HearthstoneGameBlockWrapper } from '@client/js/hearthstone/hearthstone_actions'
import { HearthstoneMatchSnapshotWrapper } from '@client/js/hearthstone/hearthstone_snapshot'
import { secondsToTimeString } from '@client/js/time'
import HearthstoneGameBlockRenderer from '@client/vue/utility/hearthstone/HearthstoneGameBlockRenderer.vue'

@Component({
    components: {
        HearthstoneGameBlockRenderer
    }
})
export default class HearthstoneTurnEventsDisplay extends Vue {
    @Prop({required: true})
    currentMatch!: HearthstoneMatchWrapper

    @Prop({type: Number, required: true})
    turn!: number

    @Prop({type:Boolean, default: false})
    disableGoto!: boolean

    @Prop({type:Boolean, default: false})
    hasVod!: boolean

    get roundName() : string {
        if (this.turn == 0) {
            return 'Mulligan'
        } else if (this.currentMatch.isBattlegrounds) {
            let actualRound = Math.floor((this.turn - 1) / 2) + 1
            return `Round ${actualRound} - ${(this.turn % 2 == 0) ? 'Battle' : 'Shop'}`
        } else {
            return `Round ${this.turn}`
        }
    }
    
    get snapshot(): HearthstoneMatchSnapshotWrapper | undefined {
        return this.currentMatch.snapshotForTurn(this.turn)
    }

    get turnTime(): Date | undefined {
        return this.snapshot?.snapshotTime
    }

    get relevantBlocks(): HearthstoneGameBlockWrapper[] {
        if (!this.snapshot) {
            return []
        }
        return this.snapshot.gameBlocks
    }

    eventStyling(e : HearthstoneGameBlockWrapper) : any {
        return {}
    }

    blockTimeDisplay(b : HearthstoneGameBlockWrapper) : string {
        return secondsToTimeString(this.currentMatch.matchTimeForBlock(b))
    }

    goToTurn() {
        this.$emit('go-to-event', this.turnTime!)
    }

    goToEvent(block: HearthstoneGameBlockWrapper) {
        this.$emit('go-to-event', block.blockTime!)
    }
}

</script>

<style scoped>

.event-list {
    overflow-y: auto;
    height: calc(100% - 73px);
}

.event-time {
    flex: 0 0 auto !important;
}

</style>