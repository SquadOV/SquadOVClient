<template>
    <div class="d-flex align-center">
        <template v-for="p in players">
            <v-tooltip
                bottom
                :key="`player-${p.summonerName}`"
            >
                <template v-slot:activator="{on, attrs}">
                    <div
                        v-on="on"
                        v-bind="attrs"
                    >
                        <lol-champion-icon
                            :class="`player-div ${sameTeam ? ((p.participantId == selfParticipantId) ? 'self-div' : 'same-team-div' ) : 'enemy-team-div' }`"
                            :width="48"
                            :height="48"
                            :game-version="gameVersion"
                            :champion-id="p.championId"
                        >
                        </lol-champion-icon>
                    </div>
                </template>

                <div class="d-flex flex-column">
                    <div class="text-h6 font-weight-bold">
                        {{ p.summonerName }}
                    </div>

                    <div class="text-subtitle-2 font-weight-bold">
                        {{ championName(p.championId) }}
                    </div>

                    <div class="text-body-2">
                        {{ p.kills }} / {{ p.deaths}} / {{ p.assists }} ({{ kda(p) }})
                    </div>

                    <div class="d-flex text-body-2">
                        <div class="d-flex align-center">
                            <img
                                class="mr-1"
                                :src="$root.generateAssetUri('assets/lol/sword_clipped.png')"
                            />
                            {{ p.totalDamageDealtToChampions }}
                        </div>

                        <div class="d-flex align-center mx-1">
                            <img
                                class="mr-1"
                                :src="$root.generateAssetUri('assets/lol/minion_clipped.png')"
                            />
                            {{ p.totalMinionsKilled }}
                        </div>

                        <div class="d-flex align-center mx-1">
                            <img
                                class="mr-1"
                                :src="$root.generateAssetUri('assets/lol/ward_clipped.png')"
                            />
                            {{ p.wardsPlaced }}
                        </div>
                    </div>
                </div>
            </v-tooltip>
        </template>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { LolMiniParticipantStats } from '@client/js/lol/matches'
import LolChampionIcon from '@client/vue/utility/lol/LolChampionIcon.vue'
import { ddragonContainer } from '@client/js/lolDdragon'

@Component({
    components: {
        LolChampionIcon,
    }
})
export default class LolMatchSummaryTeamDisplay extends Vue {
    @Prop({required: true})
    players!: LolMiniParticipantStats[]

    @Prop({required: true})
    gameVersion!: string

    @Prop({default: false})
    sameTeam!: boolean

    @Prop({required: true})
    selfParticipantId!: number

    championIdToName: { [id: number]: string | undefined } = {}

    get championName(): (id: number) => string {
        return (id: number) => {
            if (!(id in this.championIdToName)) {
                ddragonContainer.getClientForVersion(this.gameVersion).getLolChampionName(id).then((resp: string) => {
                    Vue.set(this.championIdToName, id, resp)
                }).catch((err: any) => {
                    console.log('Failed to get LoL champion splash art: ', err)
                })
                return 'Loading...'
            } else {
                return this.championIdToName[id]!
            }
        }
    }

    kda(p: LolMiniParticipantStats): string {
        if (p.deaths == 0) {
            return '∞'
        }

        let kda = (p.kills + p.assists) / p.deaths
        return kda.toFixed(2)
    }
}

</script>

<style scoped>

.player-div {
    margin-left: 2px;
    margin-right: 2px;
    border-radius: 4px;
}

.same-team-div {
    border: 2px solid #4CAF50;
}

.enemy-team-div {
    border: 2px solid #FF5252;
}

.self-div {
    border: 2px solid #FFC107;
}

</style>