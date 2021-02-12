<template>
    <div class="d-flex flex-column justify-center" :style="textStyle">
        <div class="d-flex justify-center text-h3">{{ placeString }}</div>
        <div class="d-flex justify-center text-h5" v-if="rating !== null">{{ rating }}</div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { HearthstoneMatchWrapper } from '@client/js/hearthstone/hearthstone_match'
import { getOrdinal} from '@client/js/ordinal'
@Component
export default class HearthstoneBattlegroundsGameLeaderboardPlaceDisplay extends Vue {
    @Prop({required: true})
    currentMatch!: HearthstoneMatchWrapper

    get place(): number {
        if (!this.currentMatch.currentPlayerHeroEntity) {
            return 10
        }
        return this.currentMatch.currentPlayerHeroEntity.leaderboardPlace
    }

    get placeString(): string {
        return getOrdinal(this.place)
    }

    get rating(): number | null {
        if (!this.currentMatch.currentPlayer) {
            return null
        }
        return this.currentMatch.currentPlayer.battlegroundsRating
    }

    get textStyle(): any {
        return {
            'color': (this.place > 4) ? '#FF5252' :
                (this.place > 1) ? '#4CAF50' :
                '#FFC107'
        }
    }
}
</script>