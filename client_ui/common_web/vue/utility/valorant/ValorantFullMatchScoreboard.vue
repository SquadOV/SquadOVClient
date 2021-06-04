<template>
    <div>
        <div class="pa-4 text-center">
            <span class="text-overline font-weight-bold">Scoreboard</span>
        </div>
        <v-divider></v-divider>
        <v-data-table
            :headers="headers"
            :items="items"
            :items-per-page="20"
            hide-default-footer
            id="scoreboardTable"
        >
            <template v-slot:item="{ item }">
                <tr :style="teamRowStyle(item.player)" @click="changePlayer(item.player)">
                    <td class="text-center">
                        <valorant-rank-tier :value="item.rank" :max-width-height="40">
                        </valorant-rank-tier>
                    </td>

                    <td class="text-center">
                        <div class="d-flex justify-space-around align-center">
                            <valorant-agent-icon
                                :agent="item.player._p.characterId"
                                :width-height="40"
                                circular
                            >
                            </valorant-agent-icon>

                            <span>{{ agentName(item.player._p.characterId) }}</span>
                        </div>                    
                    </td>

                    <td class="text-center">
                        {{ item.kda.kills }} / {{ item.kda.deaths }} / {{ item.kda.assists }}
                    </td>

                    <td class="text-center">
                        {{ item.score.toFixed(2) }}
                    </td>

                    <td class="text-center">
                        {{ item.damage.toFixed(2) }}
                    </td>
                </tr>
            </template>
        </v-data-table>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import {
    ValorantMatchDetailsWrapper,
    ValorantMatchPlayerWrapper,
} from '@client/js/valorant/valorant_matches_parsed'
import { getValorantContent } from '@client/js/valorant/valorant_content'
import { Color, colorFromElementTheme } from '@client/js/color'

import ValorantRankTier from '@client/vue/utility/valorant/ValorantRankTier.vue'
import ValorantAgentIcon from '@client/vue/utility/valorant/ValorantAgentIcon.vue'

@Component({
    components: {
        ValorantRankTier,
        ValorantAgentIcon,
    }
})
export default class ValorantFullMatchScoreboard extends Vue {
    @Prop({required: true})
    match!: ValorantMatchDetailsWrapper

    @Prop()
    currentPlayer! : ValorantMatchPlayerWrapper | null

    agentName(id : string) : string {
        let cnt = getValorantContent(null)
        return cnt.agentIdToName(id)
    }

    changePlayer(p : ValorantMatchPlayerWrapper) {
        this.$emit('update:currentPlayer', p)
    }

    teamRowStyle(p : ValorantMatchPlayerWrapper) : any {
        let style : any = {}

        if (p._p.puuid == this.currentPlayer?._p.puuid) {
            style['border-left'] = '5px solid var(--color-self)'
        }

        let color : string
        if (!!this.currentPlayer) {
            if (this.currentPlayer._p.teamId == p._p.teamId) {
                color = 'color-friendly'
            } else {
                color = 'color-enemy'
            }
        } else {
            if (p._p.teamId == 'Blue') {
                color = 'color-blue-team'
            } else {
                color = 'color-red-team'
            }
        }

        let rgb: Color = colorFromElementTheme(this.$parent.$el, color)
        style['background-color'] = `rgba(${rgb.r}, ${rgb.g}, ${rgb.b}, 0.5)`
        return style
    } 

    get headers() : any[] {
        let align = 'center'
        let hdr = [
            {
                text: 'Rank',
                value: 'rank',
                align: align,
            },
            {
                text: 'Player',
                value: 'player',
                align: align,
                sort: (a : any, b : any) : number => {
                    let aName = this.agentName(a._p.characterId) 
                    let bName = this.agentName(b._p.characterId)
                    if (aName < bName) {
                        return -1
                    } else if (aName > bName) {
                        return 1
                    }
                    return 0
                },
            },
            {
                text: 'KDA',
                value: 'kda',
                align: align,
                sort: (a : any, b : any) : number => {
                    let aKda = (a.kills + a.assists) / a.deaths
                    let bKda = (b.kills + b.assists) / b.deaths
                    return aKda - bKda
                },
            },
            {
                text: 'CSPR',
                value: 'score',
                align: align,
            },
            {
                text: 'DPR',
                value: 'damage',
                align: align,
            },
        ]

        return hdr
    }

    get items() : any[] {
        return this.match.getPlayersDescendingCS().map((ele : ValorantMatchPlayerWrapper) => {
            return {
                player: ele,
                rank: ele._p.competitiveTier,
                kda: {
                    kills: ele._p.stats.kills,
                    deaths: ele._p.stats.deaths, 
                    assists: ele._p.stats.assists,
                },
                score: (ele._p.stats.roundsPlayed > 0) ? ele._p.stats.score / ele._p.stats.roundsPlayed : 0,
                damage: (ele._p.stats.roundsPlayed > 0) ? this.match.getDamageDoneByPlayer(ele._p.puuid) / ele._p.stats.roundsPlayed : 0,
            }
        })
    }
}

</script>

<style scoped>

>>>#scoreboardTable table {
    border-collapse: collapse !important;
}

>>>#scoreboardTable tr {
    cursor: pointer;
}

</style>