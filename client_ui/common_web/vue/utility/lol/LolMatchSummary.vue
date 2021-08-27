<template>
    <div v-if="!!currentPlayer">
        <router-link :to="gameTo" :event="disableClick ? '' : 'click'">
            <v-sheet
                :class="`match-summary ${fill ? 'full-parent-height' : ''}`"
                rounded
                :style="style"
            >
                <v-row no-gutters :class="`${fill ? 'full-parent-height' : ''}`">
                    <!-- Champion icon -->
                    <v-col :cols="mini ? 3 : 1" align-self="center">
                        <lol-champion-icon
                            :champion-id="currentPlayer.championId"
                            :game-version="match.gameVersion"
                            :width="100"
                            :height="100"
                        >
                        </lol-champion-icon>
                    </v-col>

                    <!-- Game Info (time, type, match length) -->
                    <v-col :cols="mini ? 4 : 2" align-self="center" class="text-body-2">
                        <div class="pl-2">
                            <p>
                                {{ gameMode }} ({{ matchLength }})
                            </p>
                            
                            <p>
                                {{ dateTime }}
                            </p>
                        </div>
                    </v-col>

                    <!-- Personal Stats -->
                    <v-col :cols="mini ? 5 : 3" align-self="center" class="text-body-2">
                        <div>
                            {{ currentPlayer.kills }} / {{ currentPlayer.deaths}} / {{ currentPlayer.assists }} ({{ kda }})
                        </div>

                        <div class="d-flex">
                            <div class="d-flex align-center">
                                <v-tooltip bottom>
                                    <template v-slot:activator="{on, attrs}">
                                        <img
                                            class="mr-1"
                                            :src="$root.generateAssetUri('assets/lol/sword_clipped.png')"
                                            v-on="on"
                                            v-bind="attrs"
                                        />
                                    </template>

                                    Total Damage Dealt
                                </v-tooltip>
                                {{ currentPlayer.totalDamageDealtToChampions }}
                            </div>

                            <div class="d-flex align-center mx-1">
                                <v-tooltip bottom>
                                    <template v-slot:activator="{on, attrs}">
                                        <img
                                            class="mr-1"
                                            :src="$root.generateAssetUri('assets/lol/minion_clipped.png')"
                                            v-on="on"
                                            v-bind="attrs"
                                        />
                                    </template>

                                    Total Minions Killed
                                </v-tooltip>
                                {{ currentPlayer.totalMinionsKilled }}
                            </div>

                            <div class="d-flex align-center mx-1">
                                <v-tooltip bottom>
                                    <template v-slot:activator="{on, attrs}">
                                        <img
                                            class="mr-1"
                                            :src="$root.generateAssetUri('assets/lol/ward_clipped.png')"
                                            v-on="on"
                                            v-bind="attrs"
                                        />
                                    </template>

                                    Wards Placed
                                </v-tooltip>
                                {{ currentPlayer.wardsPlaced }}
                            </div>
                        </div>
                    </v-col>

                    <!-- Team champion match up with hoverable player information -->
                    <v-col cols="6" align-self="center" class="text-body-2" v-if="!mini">
                        <div class="d-flex justify-center align-center">
                            <lol-match-summary-team-display
                                :players="sameTeamPlayers"
                                :same-team="true"
                                :self-participant-id="currentPlayer.participantId"
                                :game-version="match.gameVersion"
                            >
                            </lol-match-summary-team-display>

                            <div class="text-overline mx-1">VS</div>

                            <lol-match-summary-team-display
                                :players="enemyTeamPlayers"
                                :same-team="false"
                                :self-participant-id="currentPlayer.participantId"
                                :game-version="match.gameVersion"
                            >
                            </lol-match-summary-team-display>
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

import Component, { mixins } from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { LolPlayerMatchSummary, LolMiniParticipantStats, getLolGameMode } from '@client/js/lol/matches'
import * as pi from '@client/js/pages'
import { standardFormatTime, secondsToTimeString } from '@client/js/time'
import { ddragonContainer } from '@client/js/lolDdragon'
import LolChampionIcon from '@client/vue/utility/lol/LolChampionIcon.vue'
import LolMatchSummaryTeamDisplay from '@client/vue/utility/lol/LolMatchSummaryTeamDisplay.vue'
import CommonComponent from '@client/vue/CommonComponent'

@Component({
    components: {
        LolChampionIcon,
        LolMatchSummaryTeamDisplay,
    }
})
export default class LolMatchSummary extends mixins(CommonComponent) {
    @Prop({required: true})
    match!: LolPlayerMatchSummary

    @Prop({required: true})
    userId!: number

    @Prop({type: Boolean, default: false})
    mini!: boolean

    @Prop({type: Boolean, default: false})
    fill!: boolean

    @Prop({type: Boolean, default: false})
    disableClick!: boolean

    @Prop()
    accessToken!: string | undefined

    backgroundUrl: string = ''

    get kda(): string {
        if (!this.currentPlayer) {
            return 'N/A'
        }

        if (this.currentPlayer.deaths == 0) {
            return '∞'
        }

        let kda = (this.currentPlayer.kills + this.currentPlayer.assists) / this.currentPlayer.deaths
        return kda.toFixed(2)
    }

    get gameTo(): any {
        return {
            name: pi.LolMatchPageId,
            params: {
                matchUuid: this.match.matchUuid,
                ...this.$route.params
            },
            query: {
                userId: this.userId,
                at: this.accessToken,
                ...this.cleanQuery
            },
        }
    }

    get dateTime(): string {
        return standardFormatTime(this.match.gameCreation)
    }

    get gameMode() : string {
        return getLolGameMode(this.match.queueId, this.match.gameMode)
    }

    get matchLength(): string {
        return secondsToTimeString(this.match.gameDuration)
    }

    get winLossColor(): string {
        return `var(--${!!this.currentPlayer?.win ? 'color-top-place' : 'color-bottom-place'})`
    }

    get style() : any {
        let style: any = {
            'border-left': `5px solid ${this.winLossColor}`,
            'background-position': 'right 0% top 30%',
            'background-size': '50% auto',
        }

        if (this.backgroundUrl.length > 0) {
            style['background-image'] = `linear-gradient(to right, #1E1E1E 0 10%, transparent), url(${this.backgroundUrl})`
        }

        return style
    }

    get currentPlayer(): LolMiniParticipantStats | undefined {
        return this.match.participants.find((ele: LolMiniParticipantStats) => ele.participantId === this.match.currentParticipantId)
    }

    get sameTeamPlayers(): LolMiniParticipantStats[] {
        return this.match.participants.filter((ele: LolMiniParticipantStats) => {
            if (!this.currentPlayer) {
                return ele.teamId === 100
            } else {
                return ele.teamId === this.currentPlayer?.teamId
            }
        })
    }

    get enemyTeamPlayers(): LolMiniParticipantStats[] {
        return this.match.participants.filter((ele: LolMiniParticipantStats) => {
            if (!this.currentPlayer) {
                return ele.teamId === 200
            } else {
                return ele.teamId !== this.currentPlayer?.teamId
            }
        })
    }

    @Watch('currentPlayer')
    refreshBackgroundImage() {
        if (!this.currentPlayer) {
            return
        }

        ddragonContainer.getClientForVersion(this.match.gameVersion).getLolChampionSplashUrl(this.currentPlayer.championId).then((resp: string) => {
            this.backgroundUrl = resp
        }).catch((err: any) => {
            console.log('Failed to get LoL champion splash art: ', err)
        })
    }

    mounted() {
        this.refreshBackgroundImage()
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