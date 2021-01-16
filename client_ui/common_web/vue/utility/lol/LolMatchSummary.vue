<template>
    <div v-if="!!currentPlayer">
        <router-link :to="gameTo">
            <v-sheet
                class="match-summary"
                rounded
                :style="style"
            >
                <v-row no-gutters>
                    <!-- Champion icon -->
                    <v-col cols="1" align-self="center">
                        <lol-champion-icon
                            :champion-id="currentPlayer.championId"
                            :game-version="match.gameVersion"
                            :width="100"
                            :height="100"
                        >
                        </lol-champion-icon>
                    </v-col>

                    <!-- Game Info (time, type, match length) -->
                    <v-col cols="2" align-self="center" class="text-body-2">
                        <div class="px-1">
                            <p>
                                {{ dateTime }}
                            </p>

                            <p>
                                
                            </p>

                            <p>
                                {{ matchLength }}
                            </p>
                        </div>
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

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { LolPlayerMatchSummary, LolMiniParticipantStats } from '@client/js/lol/matches'
import {
    getGenericWinColor,
    getGenericLossColor,
    colorToCssString,
} from '@client/js/color'
import * as pi from '@client/js/pages'
import { getOrdinal } from '@client/js/ordinal'
import { standardFormatTime, secondsToTimeString } from '@client/js/time'
import LolChampionIcon from '@client/vue/utility/lol/LolChampionIcon.vue'

@Component({
    components: {
        LolChampionIcon
    }
})
export default class LolMatchSummary extends Vue {
    @Prop({required: true})
    match!: LolPlayerMatchSummary

    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    puuid!: string

    get gameTo(): any {
        return {
            name: pi.LolMatchPageId,
            params: {
                matchUuid: this.match.matchUuid,
                ...this.$route.params
            },
            query: {
                account: this.puuid,
                userId: this.userId,
                ...this.$route.query
            },
        }
    }

    get dateTime(): string {
        return standardFormatTime(this.match.gameCreation)
    }

    get matchLength(): string {
        return secondsToTimeString(this.match.gameDuration)
    }

    get winLossColor(): string {
        return colorToCssString((!!this.currentPlayer?.win) ? getGenericWinColor() : getGenericLossColor())
    }

    get style() : any {
        return {
            'border-left': `5px solid ${this.winLossColor}`,
            'background-position': 'right',
            'background-size': 'contain',
        }
    }

    get currentPlayer(): LolMiniParticipantStats | undefined {
        return this.match.participants.find((ele: LolMiniParticipantStats) => ele.participantId == this.match.currentParticipantId)
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