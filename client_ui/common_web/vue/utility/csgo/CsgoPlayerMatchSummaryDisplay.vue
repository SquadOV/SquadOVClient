<template>
    <div>
        <router-link :to="gameTo" :event="disableClick ? '' : 'click'">
            <v-sheet
                :class="`match-summary ${fill ? 'full-parent-height' : ''}`"
                rounded
                :style="style"
            >
                <v-row no-gutters :class="`${fill ? 'full-parent-height' : ''}`">
                    <!-- Match Summary -->
                    <v-col cols="3" align-self="center">
                        <div class="pl-2">
                            <div>
                                <span class="text-h4 font-weight-bold">{{ match.map }}</span>
                                <span class="text-h6">&nbsp;({{ match.mode }})</span>
                            </div>

                            <div>
                                {{ dateTime }} ({{ matchLength }})
                            </div>
                        </div>
                    </v-col>

                    <!-- Hit tracker -->
                    <v-col cols="4" align-self="center">
                        <valorant-hit-tracker
                            :headshots="match.headshots"
                            :bodyshots="match.bodyshots"
                            :legshots="match.legshots"
                            :mini="mini"
                        >
                        </valorant-hit-tracker>
                    </v-col>

                    <!-- Match Score (Team Round Score, KDA, ADR, Mvps) -->
                    <v-col cols="3" align-self="center">
                        <div :style="wlColorStyle">
                            {{ match.friendlyRounds }} - {{ match.enemyRounds }}
                        </div>

                        <div>
                            <span class="font-weight-bold">KDA </span>{{ match.kills }} / {{ match.deaths}} / {{ match.assists }} ({{ kda }})
                        </div>

                        <div>
                            <span class="font-weight-bold">ADR </span> {{ match.damagePerRound.toFixed(1) }}
                            <span class="font-weight-bold">MVP </span> {{ match.mvps }}
                        </div>
                    </v-col>

                    <!-- Other indicators (VOD/Demo) -->
                    <div class="demo-div" v-if="match.hasDemo">
                        <v-icon color="black">
                            mdi-television-box
                        </v-icon>
                    </div>

                    <div class="vod-div" v-if="hasVod">
                        <v-icon color="black">
                            mdi-video
                        </v-icon>
                    </div>
                </v-row>
            </v-sheet>
        </router-link>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { CsgoPlayerMatchSummary } from '@client/js/csgo/summary'
import {
    getGenericWinColor,
    getGenericLossColor,
    colorToCssString
} from '@client/js/color'
import { standardFormatTime, secondsToTimeString } from '@client/js/time'
import { VodAssociation } from '@client/js/squadov/vod'
import { kda } from '@client/js/valorant/valorant_player_stats'
import { apiClient, ApiData } from '@client/js/api'
import * as pi from '@client/js/pages'
import ValorantHitTracker from '@client/vue/utility/valorant/ValorantHitTracker.vue'

@Component({
    components: {
        ValorantHitTracker
    }
})
export default class CsgoPlayerMatchSummaryDisplay extends Vue {
    @Prop({required: true})
    match!: CsgoPlayerMatchSummary

    @Prop({required: true})
    userId!: number

    @Prop({type: Boolean, default: false})
    mini!: boolean

    @Prop({type: Boolean, default: false})
    fill!: boolean

    @Prop({type: Boolean, default: false})
    disableClick!: boolean

    vod: VodAssociation | null = null

    get hasVod() : boolean {
        return !!this.vod
    }

    get gameTo(): any {
        return {
            name: pi.CsgoMatchPageId,
            params: {
                ...this.$route.params,
                matchUuid: this.match.matchUuid,
            },
            query: {
                ...this.$route.query,
                userId: this.userId,
            },
        }
    }

    get winLossColor(): string {
        return colorToCssString(this.match.winner ? getGenericWinColor() : getGenericLossColor() )
    }

    get wlColorStyle() : any {
        return {
            'color': `${this.winLossColor}`
        }
    }

    get style() : any {
        let style: any = {
            'border-left': `5px solid ${this.winLossColor}`,
            'background-position': 'right center',
            'background-size': '55% auto',
        }

        // @ts-ignore
        style['background-image'] = `linear-gradient(to right, #1E1E1E 0 10%, transparent), url(${this.$root.generateAssetUri(`assets/csgo/maps/${this.match.map}.jpg`)})`

        return style
    }

    get dateTime(): string {
        return standardFormatTime(this.match.matchStartTime)
    }

    get matchLength(): string {
        return secondsToTimeString(this.match.matchLengthSeconds)
    }

    get kda() : string {
        return kda(this.match.kills, this.match.assists, this.match.deaths).toFixed(2)
    }

    @Watch('match')
    @Watch('userId')
    refreshVod() {
        apiClient.findVodFromMatchUserId(this.match.matchUuid, this.userId).then((resp : ApiData<VodAssociation>) => {
            this.vod = resp.data
        }).catch((err : any) => {
            this.vod = null
        })
    }

    mounted() {
        this.refreshVod()
    }
}

</script>

<style scoped>

.match-summary {
    width: 100%;
    position: relative;
}

.demo-div {
    position: absolute;
    right: 0;
    top: 0;
    background-color: yellow;
    clip-path: polygon(100% 0%, 0% 0%, 100% 100%);
    padding-left: 15px;
    padding-bottom: 15px;
    border-radius: 0px 4px;
}

</style>