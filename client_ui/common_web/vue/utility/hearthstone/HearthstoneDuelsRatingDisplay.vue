<template>
    <div class="d-flex justify-center text-h3">
        <span v-if="rating !== null" :style="textStyle">{{ rating }}</span>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { HearthstoneMatchWrapper, HearthstoneGameType } from '@client/js/hearthstone/hearthstone_match'

@Component
export default class HearthstoneDuelsRatingDisplay extends Vue {
    @Prop({required: true})
    currentMatch!: HearthstoneMatchWrapper

    get rating() : number | null {
        if (!this.currentMatch.currentPlayer) {
            return null
        }

        return this.currentMatch._match.metadata.gameType == HearthstoneGameType.PvpDr ?
            this.currentMatch.currentPlayer.duelsCasualRating :
            this.currentMatch.currentPlayer.duelsHeroicRating
    }

    get textStyle(): any {
        return {}
    }
}
</script>