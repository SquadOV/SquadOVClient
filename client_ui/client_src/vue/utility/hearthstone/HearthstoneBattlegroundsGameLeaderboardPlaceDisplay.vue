<template>
    <span :style="textStyle">{{ placeString }}</span>
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

    get textStyle(): any {
        return {
            'color': (this.place > 4) ? '#FF5252' :
                (this.place > 1) ? '#4CAF50' :
                '#FFC107'
        }
    }
}
</script>