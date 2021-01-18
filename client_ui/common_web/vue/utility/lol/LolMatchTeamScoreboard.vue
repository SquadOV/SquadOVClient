<template>
    <div v-if="!!teamStats">
        <div class="d-flex full-width justify-space-between align-center text-overline px-2">
            <div class="font-weight-bold">
                <template v-if="!!currentParticipantId">
                    <span v-if="sameTeam">Your Team</span>
                    <span v-else>Enemy Team</span>
                </template>

                <template v-else>
                    <span v-if="teamId === 100">Blue Team</span>
                    <span v-else>Red Team</span>
                </template>
            </div>

            <div class="d-flex align-center">
                <v-img
                    class="mr-1"
                    :width="16"
                    :height="16"
                    :src="$root.generateAssetUri('assets/lol/sword_clipped.png')"
                    contain
                >
                </v-img>
                {{ totalKills }} / {{ totalDeaths }} / {{ totalAssists }}
            </div>

            <div class="d-flex align-center">
                <v-img
                    class="mr-1"
                    :width="16"
                    :height="16"
                    :src="$root.generateAssetUri('assets/lol/gold_clipped.png')"
                    contain
                >
                </v-img>
                {{ formatThousands(totalGoldEarned) }} ({{ formatThousands(totalGoldSpent) }})
            </div>

            <div class="d-flex align-center">
                <div class="d-flex align-center mx-1">
                    <v-img
                        class="mr-1"
                        :height="16"
                        :src="$root.generateAssetUri('assets/lol/turret_clipped.png')"
                        contain
                    >
                    </v-img>
                    {{ teamStats.towerKills }}
                </div>

                <div class="d-flex align-center mx-1">
                    <v-img
                        class="mr-1"
                        :height="16"
                        :src="$root.generateAssetUri('assets/lol/inhibitor_clipped.png')"
                        contain
                    >
                    </v-img>
                    {{ teamStats.inhibitorKills }}
                </div>
                
                <div class="d-flex align-center mx-1">
                    <v-img
                        class="mr-1"
                        :height="16"
                        :src="$root.generateAssetUri('assets/lol/minion_clipped.png')"
                        contain
                    >
                    </v-img>
                    {{ totalCreepsKilled }}
                </div>
                
                <div class="d-flex align-center mx-1">
                    <v-img
                        class="mr-1"
                        :height="16"
                        :src="$root.generateAssetUri('assets/lol/baron_clipped.png')"
                        contain
                    >
                    </v-img>
                    {{ teamStats.baronKills }}
                </div>
                
                <div class="d-flex align-center mx-1">
                    <v-img
                        class="mr-1"
                        :height="16"
                        :src="$root.generateAssetUri('assets/lol/dragon_clipped.png')"
                        contain
                    >
                    </v-img>
                    {{ teamStats.dragonKills }}
                </div>
                
                <div class="d-flex align-center mx-1">
                    <v-img
                        class="mr-1"
                        :height="16"
                        :src="$root.generateAssetUri('assets/lol/rift_herald_clipped.png')"
                        contain
                    >
                    </v-img>
                    {{ teamStats.riftHeraldKills }}
                </div>
            </div>
        </div>

        <v-data-table
            :items="teamPlayers"
            :items-per-page="5"
            hide-default-header
            hide-default-footer
            class="scoreboardTable"
        >
            <template v-slot:item="{item}">
                <tr>
                    <td class="player-column">
                        <div class="d-flex align-center">
                            <lol-champion-icon
                                class="mr-2"
                                :champion-id="item.participant.championId"
                                :game-version="match.gameVersion"
                                :width="48"
                                :height="48"
                                :style="playerChampionStyle(item)"
                            >
                            </lol-champion-icon>

                            <div>
                                <div class="d-flex align-center font-weight-bold text-caption">
                                    {{ playerName(item) }}

                                    <lol-lane-icon
                                        :lane="item.participant.timeline.lane"
                                    >
                                    </lol-lane-icon>
                                </div>

                                <div class="d-flex align-center">
                                    <lol-summoner-spell-icon
                                        :spell-id="item.participant.spell1Id"
                                        :game-version="match.gameVersion"
                                        :width-height="24"
                                    >
                                    </lol-summoner-spell-icon>

                                    <lol-summoner-spell-icon
                                        :spell-id="item.participant.spell2Id"
                                        :game-version="match.gameVersion"
                                        :width-height="24"
                                    >
                                    </lol-summoner-spell-icon>
                                </div>
                            </div>
                        </div>
                    </td>

                    <td class="level-column">
                        {{ item.participant.champLevel }}
                    </td>

                    <td class="items-column">
                        <lol-player-items
                            :stats="item.participant.stats"
                            :game-version="match.gameVersion"
                        >
                        </lol-player-items>
                    </td>

                    <td class="kda-column">
                        {{ item.participant.stats.kills }} / {{ item.participant.stats.deaths }} / {{ item.participant.stats.assists }}
                    </td>

                    <td class="gold-column">
                        <div class="d-flex align-center">
                            <v-img
                                class="mr-1"
                                :max-width="16"
                                :width="16"
                                :height="16"
                                :src="$root.generateAssetUri('assets/lol/gold_clipped.png')"
                                contain
                            >
                            </v-img>
                            {{ formatThousands(item.participant.stats.goldEarned) }} ({{ formatThousands(item.participant.stats.goldSpent) }})
                        </div>
                    </td>

                    <td class="minion-column">
                        <div class="d-flex align-center">
                            <v-img
                                class="mr-1"
                                :max-width="16"
                                :width="16"
                                :height="16"
                                :src="$root.generateAssetUri('assets/lol/minion_clipped.png')"
                                contain
                            >
                            </v-img>
                            {{ item.participant.stats.totalMinionsKilled }}
                        </div>
                    </td>

                    <td class="ward-column">
                        <div class="d-flex align-center">
                            <v-img
                                class="mr-1"
                                :max-width="16"
                                :width="16"
                                :height="16"
                                :src="$root.generateAssetUri('assets/lol/ward_clipped.png')"
                                contain
                            >
                            </v-img>
                            {{ item.participant.stats.wardsPlaced }}
                        </div>
                    </td>

                    <td class="damage-column">
                        <div class="d-flex align-center">
                            <v-img
                                class="mr-1"
                                :max-width="16"
                                :width="16"
                                :height="16"
                                :src="$root.generateAssetUri('assets/lol/double_sword_green.png')"
                                contain
                            >
                            </v-img>
                            {{ item.participant.stats.totalDamageDealtToChampions }}
                        </div>

                        <div class="d-flex align-center">
                            <v-img
                                class="mr-1"
                                :max-width="16"
                                :width="16"
                                :height="16"
                                :src="$root.generateAssetUri('assets/lol/double_sword_red.png')"
                                contain
                            >
                            </v-img>
                            {{ item.participant.stats.totalDamageTaken }}
                        </div>
                    </td>
                </tr>
            </template>
        </v-data-table>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { LolMatch, extractSameTeamPlayersFromTeamId } from '@client/js/lol/matches'
import { LolTeamStats } from '@client/js/lol/team'
import { LolParticipant, WrappedLolParticipant, LolParticipantIdentity } from '@client/js/lol/participant'
import { formatThousands } from '@client/js/lol/number'
import { Color, getGenericWinColor, getGenericLossColor, getGenericFirstPlaceColor, colorToCssString } from '@client/js/color'
import { getLolBlueTeamColor, getLolRedTeamColor } from '@client/js/lol/color'
import { ddragonContainer } from '@client/js/lolDdragon'

import LolChampionIcon from '@client/vue/utility/lol/LolChampionIcon.vue'
import LolLaneIcon from '@client/vue/utility/lol/LolLaneIcon.vue'
import LolSummonerSpellIcon from '@client/vue/utility/lol/LolSummonerSpellIcon.vue'
import LolPlayerItems from '@client/vue/utility/lol/LolPlayerItems.vue'

@Component({
    components: {
        LolChampionIcon,
        LolLaneIcon,
        LolSummonerSpellIcon,
        LolPlayerItems
    }
})
export default class LolMatchTeamScoreboard extends Vue {
    formatThousands: any = formatThousands

    @Prop({required: true})
    match!: LolMatch
    
    @Prop({required: true})
    currentParticipantId!: number | null | undefined

    @Prop({required: true})
    teamId!: number

    participantIdToName: { [id: number] : string | undefined } = {}

    get totalKills() : number {
        return this.teamPlayers.reduce((acc: number, curr: WrappedLolParticipant) => {
            return acc + curr.participant.stats.kills
        }, 0)
    }

    get totalDeaths() : number {
        return this.teamPlayers.reduce((acc: number, curr: WrappedLolParticipant) => {
            return acc + curr.participant.stats.deaths
        }, 0)
    }

    get totalAssists(): number {
        return this.teamPlayers.reduce((acc: number, curr: WrappedLolParticipant) => {
            return acc + curr.participant.stats.assists
        }, 0)
    }

    get totalGoldEarned(): number {
        return this.teamPlayers.reduce((acc: number, curr: WrappedLolParticipant) => {
            return acc + curr.participant.stats.goldEarned
        }, 0)
    }

    get totalGoldSpent(): number {
        return this.teamPlayers.reduce((acc: number, curr: WrappedLolParticipant) => {
            return acc + curr.participant.stats.goldSpent
        }, 0)
    }

    get totalCreepsKilled(): number {
        return this.teamPlayers.reduce((acc: number, curr: WrappedLolParticipant) => {
            return acc + curr.participant.stats.totalMinionsKilled
        }, 0)
    }

    get teamPlayers(): WrappedLolParticipant[] {
        return extractSameTeamPlayersFromTeamId(this.match, this.teamId)
    }

    get sameTeam(): boolean {
        return (this.match.participants.find((ele: LolParticipant) => ele.participantId === this.currentParticipantId)?.teamId === this.teamId)
    }

    get teamStats(): LolTeamStats | undefined {
        return this.match.teams.find((ele: LolTeamStats) => ele.teamId === this.teamId)
    }

    playerName(p: WrappedLolParticipant): string {
        if (!(p.participant.participantId in this.participantIdToName)) {
            if (!!p.identity?.player) {
                return p.identity.player.summonerName
            } else {
                ddragonContainer.getClientForVersion(this.match.gameVersion).getLolChampionName(p.participant.championId).then((resp: string) => {
                    Vue.set(this.participantIdToName, p.participant.participantId, resp)
                }).catch((err: any) => {
                    console.log('Failed to get LoL champion name as player name bakcup: ', err)
                })
            }
            return 'Loading...'
        }
        
        return this.participantIdToName[p.participant.participantId]!
    }

    playerChampionStyle(p: WrappedLolParticipant) : any {
        let borderColor: Color

        if (!!this.currentParticipantId) {
            if (p.participant.participantId === this.currentParticipantId) {
                borderColor = getGenericFirstPlaceColor()
            } else if (this.sameTeam) {
                borderColor = getGenericWinColor()
            } else {
                borderColor = getGenericLossColor()
            }
        } else {
            if (this.teamId === 100) {
                borderColor = getLolBlueTeamColor()
            } else {
                borderColor = getLolRedTeamColor()
            }
        }

        return {
            'border-radius': '4px',
            'border': `2px solid ${colorToCssString(borderColor)}`,
        }
    }
}

</script>

<style scoped>

>>>.scoreboardTable table {
    border-collapse: collapse !important;
}

.player-column {
    width: 20%;
}

.level-column {
    width: 5%;
}

.items-column {
    width: 30%;
}

.kda-column {
    width: 10%;
}

.gold-column {
    width: 15%;
}

.minion-column {
    width: 5%;
}

.ward-column {
    width: 5%;
}

.damage-column {
    width: 10%;
}

</style>