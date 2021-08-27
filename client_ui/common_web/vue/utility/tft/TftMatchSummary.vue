<template>
    <div>
        <router-link :to="gameTo" :event="disableClick ? '' : 'click'">
            <v-sheet
                :class="`match-summary ${fill ? 'full-parent-height' : ''}`"
                rounded
                :style="style"
            >
                <v-row no-gutters :class="`${fill ? 'full-parent-height' : ''}`">
                    <!-- Little legend icon -->
                    <v-col :cols="mini ? 3 : 1" align-self="center">
                        <tft-little-legend-icon
                            :content-id="match.companion.contentId"
                            :width="100"
                            :height="100"
                        >
                        </tft-little-legend-icon>
                    </v-col>

                    <!-- Placement and Traits -->
                    <v-col :cols="mini ? 5 : 2" align-self="center">
                        <div class="d-flex flex-column justify-center" :style="placementStyle">
                            <div class="d-flex justify-center text-h3">{{ placementString }}</div>
                            <tft-full-trait-display
                                class="mx-2"
                                :traits="match.traits"
                                :tft-set="patch"
                            >
                            </tft-full-trait-display>
                        </div>
                    </v-col>

                    <!-- Game Info (time, type, match length) -->
                    <v-col :cols="mini ? 4 : 2" align-self="center" class="text-body-2">
                        <p>
                            {{ dateTime }}
                        </p>

                        <p>
                            {{ queueType }} (Set {{ patch }})
                        </p>

                        <p>
                            {{ matchLength }} - {{ match.lastRound }} Rounds
                        </p>
                    </v-col>

                    <!-- Units -->
                    <v-col cols="7" align-self="center" v-if="!mini">
                        <tft-full-unit-display
                            class="mx-2 my-1"
                            :units="match.units"
                            :tft-set="patch"
                            :single-width="48"
                        >
                        </tft-full-unit-display>
                    </v-col>
                </v-row>

                <div class="vod-div" v-if="match.hasVod">
                    <v-icon color="black">
                        mdi-video
                    </v-icon>
                </div>
            </v-sheet>
        </router-link>
    </div>
</template>

<script lang="ts">

import Component, { mixins } from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { TftPlayerMatchSummary, getTftSetNumber } from '@client/js/tft/matches'
import * as pi from '@client/js/pages'
import { getOrdinal } from '@client/js/ordinal'
import { standardFormatTime, secondsToTimeString } from '@client/js/time'
import { TFT_RANKED_QUEUE_ID } from '@client/js/tft/queue'

import TftLittleLegendIcon from '@client/vue/utility/tft/TftLittleLegendIcon.vue'
import TftFullTraitDisplay from '@client/vue/utility/tft/TftFullTraitDisplay.vue'
import TftFullUnitDisplay from '@client/vue/utility/tft/TftFullUnitDisplay.vue'
import CommonComponent from '@client/vue/CommonComponent'

@Component({
    components: {
        TftLittleLegendIcon,
        TftFullTraitDisplay,
        TftFullUnitDisplay
    }
})
export default class TftMatchSummary extends mixins(CommonComponent) {
    @Prop({required: true})
    match!: TftPlayerMatchSummary

    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    puuid!: string

    @Prop({type: Boolean, default: false})
    mini!: boolean

    @Prop({type: Boolean, default: false})
    fill!: boolean

    @Prop({type: Boolean, default: false})
    disableClick!: boolean

    @Prop()
    accessToken!: string | undefined

    get gameTo(): any {
        return {
            name: pi.TftMatchPageId,
            params: {
                ...this.$route.params,
                matchUuid: this.match.matchUuid,
            },
            query: {
                ...this.cleanQuery,
                account: this.puuid,
                userId: this.userId,
                at: this.accessToken,
            },
        }
    }

    get dateTime(): string {
        return standardFormatTime(this.match.gameDatetime)
    }

    get queueType(): string {
        return (this.match.queueId == TFT_RANKED_QUEUE_ID) ? 'Ranked' : 'Normal'
    }

    get matchLength(): string {
        return secondsToTimeString(this.match.gameLength)
    }

    get patch(): string {
        return getTftSetNumber(this.match.tftSetNumber, this.match.gameVersion)
    }

    get winLossColor(): string {
        return (this.match.placement > 4) ? 'color-bottom-place' :
            (this.match.placement > 1) ? 'color-top-place' :
            'color-first-place'
    }

    get style() : any {
        return {
            'border-left': `5px solid var(--${this.winLossColor})`,
            'background-position': 'right',
            'background-size': 'contain',
        }
    }

    get placementString(): string {
        return getOrdinal(this.match.placement)
    }

    get placementStyle(): any {
        return {
            'color': `var(--${this.winLossColor})`
        }
    }
}

</script>

<style scoped>

.match-summary {
    width: 100%;
    position: relative;
}

p {
    margin-bottom: 2px !important;
}

</style>