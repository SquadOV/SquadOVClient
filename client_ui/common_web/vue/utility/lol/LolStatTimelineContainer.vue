<template>
    <div>
        <div class="d-flex align-center my-1">
            <v-select
                class="item-select"
                v-model="selection"
                dense
                solo
                label="Who"
                hide-details
                :items="selectOptions"
                :multiple="!diffGraphs"
            >
                <template v-slot:item="{item}">
                    <div class="d-flex align-center">
                        <template v-if="!diffGraphs">
                            <v-checkbox
                                class="selection-checkbox"
                                dense
                                hide-details
                                :input-value="selection.includes(item.value)"
                                readonly
                            >
                            </v-checkbox>
                        </template>

                        <template v-if="!!item.player">
                            <lol-participant-display
                                :participant="item.player"
                                :match="match"
                                :width-height="32"
                                :current-participant-id="currentParticipantId"
                                no-tooltip
                            >
                            </lol-participant-display>
                        </template>

                        <template v-else>
                            {{ item.text }}
                        </template>
                    </div>
                </template>

                <template v-slot:selection="{item}">
                    <div class="mx-1">
                        <template v-if="!!item.player">
                            <lol-participant-display
                                :participant="item.player"
                                :match="match"
                                :width-height="32"
                                :current-participant-id="currentParticipantId"
                                no-tooltip
                            >
                            </lol-participant-display>
                        </template>

                        <template v-else>
                            <v-chip>
                                {{ item.text }}
                            </v-chip>
                        </template>
                    </div>
                </template>
            </v-select>

            <v-spacer></v-spacer>

            <v-dialog
                v-model="showSettings"
                max-width="60%"
            >
                <template v-slot:activator="{on}">
                    <v-btn
                        v-on="on"
                        icon
                    >
                        <v-icon>
                            mdi-cog
                        </v-icon>
                    </v-btn>
                </template>

                <v-card>
                    <v-card-title>
                        Graph Settings
                    </v-card-title>
                    <v-divider></v-divider>

                    <div class="d-flex flex-wrap justify-space-around ma-1">
                        <v-checkbox
                            class="mx-2"
                            v-model="teamGraphs"
                            label="Team"
                            dense
                            hide-details
                        >
                        </v-checkbox>

                        <v-checkbox
                            class="mx-2"
                            v-model="diffGraphs"
                            label="Diffs"
                            dense
                            hide-details
                        >
                        </v-checkbox>
                    </div>

                    <v-card-actions>
                        <v-spacer></v-spacer>
                        <v-btn
                            color="success"
                            @click="showSettings = false"
                        >
                            Close
                        </v-btn>
                    </v-card-actions>
                </v-card>
            </v-dialog>
        </div>

        <v-tabs vertical v-model="currentTab">
            <v-tab>
                Gold
            </v-tab>

            <v-tab-item>
                <div class="full-width graph pa-1">
                    <line-graph
                        ref="goldGraph"
                        :series-data="goldSeries"
                        :forced-min-x="0"
                        :forced-max-x="match.gameDuration"
                        :diff-graph="diffGraphs"
                        @graphclick="handleClick"
                    >
                    </line-graph>
                </div>
            </v-tab-item>

            <v-tab>
                XP
            </v-tab>

            <v-tab-item>
                <div class="full-width graph pa-1">
                    <line-graph
                        ref="xpGraph"
                        :series-data="xpSeries"
                        :forced-min-x="0"
                        :forced-max-x="match.gameDuration"
                        :diff-graph="diffGraphs"
                        @graphclick="handleClick"
                    >
                    </line-graph>
                </div>
            </v-tab-item>

            <v-tab>
                Level
            </v-tab>

            <v-tab-item>
                <div class="full-width graph pa-1">
                    <line-graph
                        ref="levelGraph"
                        :series-data="levelSeries"
                        :forced-min-x="0"
                        :forced-max-x="match.gameDuration"
                        :diff-graph="diffGraphs"
                        @graphclick="handleClick"
                    >
                    </line-graph>
                </div>
            </v-tab-item>

            <v-tab>
                Minions
            </v-tab>

            <v-tab-item>
                <div class="full-width graph pa-1">
                    <line-graph
                        ref="minionGraph"
                        :series-data="minionSeries"
                        :forced-min-x="0"
                        :forced-max-x="match.gameDuration"
                        :diff-graph="diffGraphs"
                        @graphclick="handleClick"
                    >
                    </line-graph>
                </div>
            </v-tab-item>
        </v-tabs>
    </div>

</template>

<script lang="ts">

import Vue from 'vue'
import Component, { mixins } from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import {
    LolMatch,
    getTeamIdFromParticipantId,
    extractSameTeamPlayersFromParticipantId,
    extractEnemyTeamPlayersFromParticipantId,
    extractSameTeamPlayersFromTeamId,
    extractLolFrameStatsFromTimelineForPlayer,
    extractLolFrameStatsFromTimelineForTeam,
    getLolOpposingPlayer
} from '@client/js/lol/matches'
import { LolMatchTimeline, LolMatchFrameStat, LolMatchFrame, getLolMatchNonEmptyTimelineFrames } from '@client/js/lol/timeline'
import { LolTeamStats, lolTeamIdToString, getOpposingLolTeam } from '@client/js/lol/team'
import { WrappedLolParticipant } from '@client/js/lol/participant'
import { StatXYSeriesData } from '@client/js/stats/seriesData'
import { ddragonContainer } from '@client/js/lolDdragon'
import { colorToCssString, colorFromElementTheme } from '@client/js/color'
import LolParticipantDisplay from '@client/vue/utility/lol/LolParticipantDisplay.vue'
import LineGraph from '@client/vue/stats/LineGraph.vue'
import CommonComponent from '@client/vue/CommonComponent'

@Component({
    components: {
        LolParticipantDisplay,
        LineGraph
    }
})
export default class LolStatTimelineContainer extends mixins(CommonComponent) {
    @Prop({required: true})
    match!: LolMatch

    @Prop({required: true})
    timeline!: LolMatchTimeline

    @Prop({required: true})
    currentParticipantId!: number | null | undefined

    @Prop({required: true})
    currentTime!: number

    showSettings: boolean = false
    teamGraphs: boolean = false
    diffGraphs: boolean = false
    selection: number[] | number = []
    participantIdToName: { [id: number] : string | undefined } = {}
    currentTab: number = 0

    $refs!: {
        goldGraph: LineGraph
        xpGraph: LineGraph
        levelGraph: LineGraph
        minionGraph: LineGraph
    }

    @Watch('currentTab')
    refreshGraphs() {
        Vue.nextTick(() => {
            if (this.currentTab == 0 && !!this.$refs.goldGraph) {
                this.$refs.goldGraph.graphResize()
            } else if (this.currentTab == 1 && !!this.$refs.xpGraph) {
                this.$refs.xpGraph.graphResize()
            } else if (this.currentTab == 2 && !!this.$refs.levelGraph) {
                this.$refs.levelGraph.graphResize()
            } else if (!!this.$refs.minionGraph) {
                this.$refs.minionGraph.graphResize()
            }
        })
    }

    get currentTeamId(): number | undefined {
        if (!this.currentParticipantId) {
            return undefined
        }
        return getTeamIdFromParticipantId(this.match, this.currentParticipantId)
    }

    get playerName(): (p: WrappedLolParticipant) => string {
        let match = this.match
        let participantIdToName = this.participantIdToName

        return (p: WrappedLolParticipant): string => {
            if (!(p.participant.participantId in participantIdToName)) {
                if (!!p.identity?.player) {
                    return p.identity.player.summonerName
                } else {
                    ddragonContainer.getClientForVersion(match.gameVersion).getLolChampionName(p.participant.championId).then((resp: string) => {
                        Vue.set(this.participantIdToName, p.participant.participantId, resp)
                    }).catch((err: any) => {
                        console.error('Failed to get LoL champion name as player name bakcup: ', err)
                    })
                }
                return 'Loading...'
            }
            
            return participantIdToName[p.participant.participantId]!
        }
    }

    get selectOptions(): any[] {
        if (this.teamGraphs) {
            return this.match.teams.map((ele: LolTeamStats) => {
                return {
                    text: !!this.currentTeamId ?
                        ((this.currentTeamId === ele.teamId) ? 'Your Team': 'Enemy Team') :
                        lolTeamIdToString(ele.teamId),
                    value: ele.teamId
                }
            })
        } else {
            let allPlayers: WrappedLolParticipant[] = []
            if (!!this.currentParticipantId && !!this.currentTeamId) {
                allPlayers = [
                    ...extractSameTeamPlayersFromParticipantId(this.match, this.currentParticipantId)!,
                    ...extractEnemyTeamPlayersFromParticipantId(this.match, this.currentParticipantId)!
                ]
            } else {
                allPlayers = [
                    ...extractSameTeamPlayersFromTeamId(this.match, 100),
                    ...extractSameTeamPlayersFromTeamId(this.match, 200),
                ]
            }
            return allPlayers.map((ele: WrappedLolParticipant) => {
                return {
                    player: ele,
                    text: this.playerName(ele),
                    value: ele.participant.participantId
                }
            })
        }
    }

    @Watch('teamGraphs')
    resetSelectionTeam() {
        this.selection = []
        if (this.teamGraphs) {
            if (!this.diffGraphs) {
                this.selection = [100, 200]
            } else {
                this.selection = 100
            }
        } else if (!!this.currentParticipantId) {
            if (!this.diffGraphs) {
                this.selection = [this.currentParticipantId]
            } else {
                this.selection = this.currentParticipantId
            }
        }
    }

    @Watch('diffGraphs')
    resetSelectionDiff() {
        if (this.diffGraphs && (<number[]>this.selection).length > 0) {
            this.selection = (<number[]>this.selection)[0]
        } else if (!this.diffGraphs) {
            this.selection = [<number>this.selection]
        }
    }

    get selectionArray(): number[] {
        if (this.diffGraphs) {
            return [<number>this.selection]
        } else {
            return <number[]>this.selection
        }
    }

    get computeGraphSeries(): (st: LolMatchFrameStat) => StatXYSeriesData[] {
        let match = this.match
        let timeline = this.timeline
        let ids = this.selectionArray
        let diffGraph = this.diffGraphs
        let teamGraphs = this.teamGraphs

        return  (st: LolMatchFrameStat): StatXYSeriesData[] => {
            let x: number[] = [
                ...getLolMatchNonEmptyTimelineFrames(timeline).map((ele: LolMatchFrame) => ele.timestamp / 1000),
                this.match.gameDuration,
            ]
            return ids.map((id: number) => {
                let y: number[] = []

                if (teamGraphs) {
                    y = extractLolFrameStatsFromTimelineForTeam(match, timeline, st, id)
                } else {
                    y = extractLolFrameStatsFromTimelineForPlayer(match, timeline, st, id)
                }

                if (diffGraph) {
                    let diffY : number[] = []
                    if (teamGraphs) {
                        let opposingId = getOpposingLolTeam(id)
                        diffY = extractLolFrameStatsFromTimelineForTeam(match, timeline, st, opposingId)
                    } else {
                        let opposingId = getLolOpposingPlayer(match, timeline, id)
                        if (!!opposingId) {
                            diffY = extractLolFrameStatsFromTimelineForPlayer(match, timeline, st, opposingId)
                        }
                    }

                    if (y.length == diffY.length) {
                        y = y.map((val: number, idx: number) => {
                            return val - diffY[idx]
                        })
                    } else {
                        console.warn('Diff Graph can not be populated due to difference in data length.')
                    }
                }

                let data = new StatXYSeriesData(
                    x,
                    y,
                    'value',
                    'elapsedSeconds',
                    this.selectOptions.find((ele: any) => ele.value === id)?.text
                )

                data.addXMarkLine({
                    x: this.currentTime,
                    name: '',
                    symbol: 'none',
                    colorOverride: colorToCssString(colorFromElementTheme(this.$parent.$el, 'color-self')),
                })

                return data
            })
        }
    }

    get goldSeries(): StatXYSeriesData[] {
        return this.computeGraphSeries(LolMatchFrameStat.EGold)
    }

    get xpSeries(): StatXYSeriesData[] {
        return this.computeGraphSeries(LolMatchFrameStat.EXp)
    }

    get levelSeries(): StatXYSeriesData[] {
        return this.computeGraphSeries(LolMatchFrameStat.ELevel)
    }

    get minionSeries(): StatXYSeriesData[] {
        return this.computeGraphSeries(LolMatchFrameStat.EMinions)
    }

    handleClick(evt: {gridIndex: number, pts: number[]}) {
        let tm = evt.pts[0] * 1000.0
        this.sendAnalyticsEvent(this.AnalyticsCategory.MatchInfo, this.AnalyticsAction.GoToTimeline, '', tm)
        this.$emit('go-to-timestamp', tm)
    }

    mounted() {
        if (!!this.currentParticipantId) {
            this.selection = [this.currentParticipantId]
        } else {
            this.teamGraphs = true
            this.selection = [100, 200]
        }

        this.refreshGraphs()
    }
}

</script>

<style scoped>

.selection-checkbox {
    margin: 0px !important;
}

.item-select {
    flex-grow: 0 !important;
    min-width: 200px;
}

.graph {
    height: 650px;
}

>>>.v-select__selections input { display: none }

</style>