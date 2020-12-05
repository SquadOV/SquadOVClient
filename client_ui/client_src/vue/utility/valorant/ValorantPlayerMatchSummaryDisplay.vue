<template>
    <div>
        <router-link :to="gameTo">
            <v-sheet
                class="match-summary"
                rounded
                :style="style"
            >
                <v-row no-gutters>
                    <v-col cols="2" align-self="center">
                        <valorant-agent-icon
                            :agent="match.characterId"
                            :width-height="100"
                            :patch="match.gameVersion"
                        >
                        </valorant-agent-icon>
                    </v-col>

                    <v-col cols="1" align-self="center">
                        <p :style="wlColorStyle">
                            {{ match.roundsWon }} - {{ match.roundsLost }}
                        </p>

                        <v-chip :style="csRankStyle">{{ csRank }}</v-chip>
                    </v-col>

                    <v-col cols="4" align-self="center">
                        <valorant-hit-tracker
                            :headshots="match.headshots"
                            :bodyshots="match.bodyshots"
                            :legshots="match.legshots"
                        >
                        </valorant-hit-tracker>
                    </v-col>

                    <v-col cols="3" align-self="center">
                        <div>
                            {{ match.kills }} / {{ match.deaths}} / {{ match.assists }} ({{ kda }})
                        </div>

                        <div>
                            <span class="stat-label">DPR </span> {{ dpr }}
                            <span class="stat-label">CSPR </span> {{ cspr }}
                        </div>
                    </v-col>
                </v-row>

                <!-- Game Type (Map) in top right corner -->
                <div class="mode-div text-overline" :style="queueTypeStyle">
                    {{ queueType }} ({{ mapName }})
                </div>

                <!-- VOD presence indicator in bottom right corner-->
                <div class="vod-div" v-if="hasVod">
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
import { ValorantPlayerMatchSummary, getGameMode, getIsCustom } from '@client/js/valorant/valorant_matches'
import { kda, dpr, cspr } from '@client/js/valorant/valorant_player_stats'
import { getOrdinal } from '@client/js/ordinal'
import { getValorantContent } from '@client/js/valorant/valorant_content'
import { VodAssociation } from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import ValorantAgentIcon from '@client/vue/utility/valorant/ValorantAgentIcon.vue'
import ValorantHitTracker from '@client/vue/utility/valorant/ValorantHitTracker.vue'
import * as pi from '@client/js/pages'

@Component({
    components: {
        ValorantAgentIcon,
        ValorantHitTracker
    }
})
export default class ValorantPlayerMatchSummaryDisplay extends Vue {
    @Prop({required: true})
    match! : ValorantPlayerMatchSummary

    @Prop({required: true})
    userId!: number

    vod: VodAssociation | null = null

    get hasVod() : boolean {
        return !!this.vod
    }

    get gameTo(): any {
        return {
            name: pi.ValorantMatchPageId,
            params: {
                matchId: this.match.matchId,
                ...this.$route.params
            },
            query: {
                account: this.$route.params.account,
                userId: this.userId,
                ...this.$route.query
            },
        }
    }

    get csRank() : string {
        return getOrdinal(this.match.combatScoreRank)
    }

    get csRankStyle() : any {
        let color = '#424242'
        if (this.match.combatScoreRank == 1) {
            color = '#AF9500'
        } else if (this.match.combatScoreRank == 2) {
            color = '#6C7A86'
        } else if (this.match.combatScoreRank == 3) {
            color = '#AD8A56'
        }
        return {
            'background-color': color
        }
    }

    get winLossColor() : string {
        let color : string = !!this.match.won ? '#4CAF50' : '#FF5252'
        return color
    }

    get queueType() : string {
        let queue = getGameMode(this.match.gameVersion, this.match.gameMode, this.match.isRanked)
        if (!queue || !this.match.provisioningFlowID) {
            return 'Unknown'
        }

        if (getIsCustom(this.match.provisioningFlowID)) {
            queue = `[Custom] ${queue}`
        }
        return queue
    }

    get queueTypeStyle() : any {
        let color = ''
        let queue = getGameMode(this.match.gameVersion, this.match.gameMode, this.match.isRanked)
        if (!!this.match.provisioningFlowID && getIsCustom(this.match.provisioningFlowID)) {
            color = '#343a40'
        } else if (this.match.isRanked) {
            color = '#6c757d'
        } else if (queue == 'Unrated') {
            color = '#007bff'
        } else if (queue == 'Deathmatch') {
            color = '#17a2b8'
        } else if (queue == 'Spike Rush') {
            color = '#28a745'
        } else {
            color = '#ff0000'
        }

        return {
            'background-color': color,
        }
    }

    get mapName() : string {
        if (!this.match.map) {
            return 'Unknown'
        }

        let cnt = getValorantContent(this.match.gameVersion)
        return cnt.mapAssetPathToName(this.match.map)
    }

    get style() : any {
        let style: any = {
            'border-left': `5px solid ${this.winLossColor}`,
            'background-position': 'right',
            'background-size': 'contain',
        }

        if (!!this.match.gameVersion && !!this.match.map) {
            style['background-image'] = `linear-gradient(to right, #1E1E1E 0 70%, transparent), url(assets/valorant/maps/preview/${this.mapName}.png)`
        }

        return style
    }

    get wlColorStyle() : any {
        return {
            'color': `${this.winLossColor}`
        }
    }

    get kda() : string {
        return kda(this.match.kills, this.match.assists, this.match.deaths).toFixed(2)
    }

    get dpr() : string {
        return dpr(this.match.totalDamage, this.match.roundsPlayed).toFixed(2)
    }

    get cspr() : string {
        return cspr(this.match.totalCombatScore, this.match.roundsPlayed).toFixed(2)
    }

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

.stat-label {
    font-weight: bold;
}

.mode-div {
    position: absolute;
    right: 0;
    top: 0;
    border-radius: 0px 4px 0px 10px;
    padding: 4px;
    line-height: 1.2rem !important;
}

</style>