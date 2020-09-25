<template>
    <v-sheet
        class="match-summary"
        rounded
        :style="style"
    >
        <v-row no-gutters>
            <v-col cols="2" align-self="center">
                <valorant-agent-icon
                    :agent="match.agentId"
                    :width-height="100"
                    :patch="match.patchId"
                >
                </valorant-agent-icon>
            </v-col>

            <v-col cols="1" align-self="center">
                <p :style="wlColorStyle">
                    {{ match.myTeamScore }} - {{ match.otherTeamScore }}
                </p>

                <v-chip :style="csRankStyle">{{ csRank }}</v-chip>
            </v-col>

            <v-col cols="4" align-self="center">
                <valorant-hit-tracker
                    :headshots="match.stats.headshots"
                    :bodyshots="match.stats.bodyshots"
                    :legshots="match.stats.legshots"
                >
                </valorant-hit-tracker>
            </v-col>

            <v-col cols="3" align-self="center">
                <div>
                    {{ match.stats.kills }} / {{ match.stats.deaths}} / {{ match.stats.assists }} ({{ kda }})
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
        <div class="vod-div" v-if="match.hasVod">
            <v-icon color="black">
                mdi-video
            </v-icon>
        </div>
    </v-sheet>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { ValorantPlayerMatchSummary, getGameMode, getIsCustom } from '@client/js/valorant/valorant_matches'
import { kda, dpr, cspr } from '@client/js/valorant/valorant_player_stats'
import { getOrdinal } from '@client/js/ordinal'
import { getValorantContent } from '@client/js/valorant/valorant_content'

import ValorantAgentIcon from '@client/vue/utility/valorant/ValorantAgentIcon.vue'
import ValorantHitTracker from '@client/vue/utility/valorant/ValorantHitTracker.vue'

@Component({
    components: {
        ValorantAgentIcon,
        ValorantHitTracker
    }
})
export default class ValorantPlayerMatchSummaryDisplay extends Vue {
    @Prop({required: true})
    match! : ValorantPlayerMatchSummary

    get csRank() : string {
        return getOrdinal(this.match.csRank)
    }

    get csRankStyle() : any {
        let color = '#424242'
        if (this.match.csRank == 1) {
            color = '#AF9500'
        } else if (this.match.csRank == 2) {
            color = '#6C7A86'
        } else if (this.match.csRank == 3) {
            color = '#AD8A56'
        }
        return {
            'background-color': color
        }
    }

    get winLossColor() : string {
        let color : string = this.match.win ? '#4CAF50' : '#FF5252'
        return color
    }

    get queueType() : string {
        let queue = getGameMode(this.match)
        if (getIsCustom(this.match)) {
            queue = `[Custom] ${queue}`
        }
        return queue
    }

    get queueTypeStyle() : any {
        let color = ''
        let queue = getGameMode(this.match)
        if (getIsCustom(this.match)) {
            color = '#343a40'
        } else if (this.match.isRanked) {
            color = '#28a745'
        } else if (queue == 'Unrated') {
            color = '#007bff'
        } else if (queue == 'Deathmatch') {
            color = '#17a2b8'
        } else if (queue == 'Spike Rush') {
            color = '#6c757d'
        }

        return {
            'background-color': color,
        }
    }

    get mapName() : string {
        let cnt = getValorantContent(this.match.patchId)
        return cnt.mapAssetPathToName(this.match.map)
    }

    get style() : any {
        return {
            'border-left': `5px solid ${this.winLossColor}`,
            'background-image': `linear-gradient(to right, #1E1E1E 0 70%, transparent), url(assets/valorant/maps/preview/${this.mapName}.png)`,
            'background-position': 'right',
            'background-size': 'contain',
        }
    }

    get wlColorStyle() : any {
        return {
            'color': `${this.winLossColor}`
        }
    }

    get kda() : string {
        return kda(this.match.stats).toFixed(2)
    }

    get dpr() : string {
        return dpr(this.match.stats).toFixed(2)
    }

    get cspr() : string {
        return cspr(this.match.stats).toFixed(2)
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

.vod-div {
    position: absolute;
    right: 0;
    bottom: 0;
    background-color: yellow;
    clip-path: polygon(0 100%, 100% 0, 100% 100%);
    padding-left: 15px;
    padding-top: 15px;
    border-radius: 0px 4px;
}

</style>