<template>
    <div class="d-flex flex-column" :style="topDivStyle" v-if="!!medalInfo">
        <div class="d-flex justify-center">
            <v-img
                :src="starImage"
                :width="starWidthHeight"
                :height="starWidthHeight"
                contain
                v-for="i in numStars"
                :key="`star-${i}`"
            >
            </v-img>
        </div>

        <div :style="imageDivStyle"> 
            <v-img
                :src="frameImage"
                :width="maxHeight"
                :height="maxHeight"
                contain
                position="top"
                :style="frameStyle"
            >
            </v-img>

            <v-img
                :src="portraitImage"
                :width="maxHeight"
                :height="maxHeight"
                contain
                position="top"
                :style="portraitStyle"
            >
            </v-img>
        </div>

        <div class="d-flex justify-center">
            {{ rankName }}
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { HearthstoneMedalInfo } from '@client/js/hearthstone/hearthstone_player'
import { HearthstoneRank, earnedStarsToHearthstoneRank,  earnedStarsToHearthstoneSubrank, hearthstoneRankToString } from '@client/js/hearthstone/hearthstone_rank'

@Component
export default class HearthstonePlayerMedalDisplay extends Vue {
    @Prop({required: true})
    medalInfo!: HearthstoneMedalInfo | undefined

    @Prop({default: 128})
    maxHeight!: number

    get rank() : HearthstoneRank {
        return earnedStarsToHearthstoneRank(this.medalInfo!.starLevel)
    }

    get rankName() : string {
        return `${this.tier} ${this.rankSubtier}`
    }

    get rankSubtier() : number {
        return earnedStarsToHearthstoneSubrank(this.medalInfo!.earnedStars, this.medalInfo!.legendIndex)
    }

    get frameImage(): string {
        return `assets/hearthstone/borders/Ranked_Medal_Frame_${this.tier}.png`
    }

    get portraitImage(): string {
        return `assets/hearthstone/ranks/RankedPlay_Medal_Portrait_${this.tier}_${`${this.rankSubtier}`.padStart(2, '0')}.png`
    }

    get maskImage(): string {
        return `assets/hearthstone/masks/rank_portrait_mask.png`
    }

    get starImage(): string {
        return `assets/hearthstone/Star.png`
    }

    get starWidthHeight() : number {
        let mul = this.maxHeight / 512.0
        return 128 * mul
    }

    get numStars() : number {
        return this.medalInfo!.earnedStars
    }

    get isLegend() : boolean {
        return this.rank == HearthstoneRank.Legend
    }

    get tier() : string {
        return hearthstoneRankToString(this.rank)
    }

    get topDivStyle() : any {
        return {
            'min-width': `${this.maxHeight}px`,
        }
    }

    get imageDivStyle() : any {
        return {
            'width': `${this.maxHeight}px`,
            'height': `${this.maxHeight}px`,
            'position': 'relative'
        }
    }

    get frameStyle(): any {
        return {
            'position': 'absolute',
            'z-index': 2
        }
    }

    get portraitStyle(): any {
        return {
            'position': 'absolute',
            'z-index': 1,
            'mask-image': `url(${this.maskImage})`,
            'mask-size': 'contain'
        }
    }
}

</script>