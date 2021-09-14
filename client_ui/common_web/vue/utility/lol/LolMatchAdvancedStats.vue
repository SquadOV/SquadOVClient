<template>
    <table>
        <thead>
            <tr>
                <th>
                    Stat
                </th>

                <th
                    v-for="(p, idx) in allPlayers"
                    :key="`header-${idx}`"
                >
                    <div class="d-flex flex-column align-center justify-center my-1">
                        <v-tooltip bottom>
                            <template v-slot:activator="{on, attrs}">
                                <div
                                    v-on="on"
                                    v-bind="attrs"
                                >
                                    <lol-champion-icon
                                        :style="playerChampionStyle(p)"
                                        :champion-id="p.championId"
                                        :game-version="match.info.gameVersion"
                                        :width="32"
                                        :height="32"
                                    >
                                    </lol-champion-icon>
                                </div>
                            </template>
                            
                            <div class="font-caption font-weight-bold">
                                {{ playerName(p) }}
                            </div>
                        </v-tooltip>
                    </div>
                </th>
            </tr>
        </thead>

        <template v-for="(g, idx) in groups">
            <tr
                class="group-row"
                :key="`group-${idx}`"
            >
                <td :colspan="allPlayers.length + 1">
                    <div class="d-flex align-center group-field">
                        <v-btn
                            icon
                            x-small
                            @click="toggleGroupOpen(g)"
                        >
                            <v-icon v-if="isGroupExpanded[lolStatCategoryToName(g)]">
                                mdi-minus
                            </v-icon>

                            <v-icon v-else>
                                mdi-plus
                            </v-icon>
                        </v-btn>

                        <span class="font-weight-bold">{{ lolStatCategoryToName(g) }}</span>
                    </div>
                </td>
            </tr>

            <template v-if="isGroupExpanded[lolStatCategoryToName(g)]">
                <tr
                    v-for="(st, stIdx) in getLolDisplayStatsForCategory(g)"
                    :key="`stat-${idx}-${stIdx}`"
                >
                    <td>
                        <div class="d-flex align-center stat-field text-overline">
                            {{ lolDisplayStatToName(st) }}
                        </div>
                    </td>

                    <td
                        class="text-center"
                        v-for="(p, pIdx) in allPlayers"
                        :key="`player-${idx}-${stIdx}-${pIdx}`"
                    >
                        <lol-advanced-stat-display
                            :value="extractLolDisplayStatFromParticipant(p, st)"
                        >
                        </lol-advanced-stat-display>
                    </td>
                </tr>
            </template>
        </template>
    </table>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import {
    LolMatch,
    extractSameTeamPlayersFromParticipantId,
    extractEnemyTeamPlayersFromParticipantId,
    extractSameTeamPlayersFromTeamId,
    getTeamIdFromParticipantId
} from '@client/js/lol/matches'
import {
    LolParticipant,
    lolDisplayStatToName,
    extractLolDisplayStatFromParticipant,
    allLolStatCategories,
    getLolDisplayStatsForCategory,
    ELolStatCategory,
    lolStatCategoryToName
} from '@client/js/lol/participant'
import LolChampionIcon from '@client/vue/utility/lol/LolChampionIcon.vue'
import { ddragonContainer } from '@client/js/lolDdragon'
import { getLolBlueTeamColor, getLolRedTeamColor } from '@client/js/lol/color'
import LolAdvancedStatDisplay from '@client/vue/utility/lol/LolAdvancedStatDisplay.vue'

@Component({
    components: {
        LolChampionIcon,
        LolAdvancedStatDisplay,
    }
})
export default class LolMatchAdvancedStats extends Vue {
    getLolDisplayStatsForCategory = getLolDisplayStatsForCategory
    lolDisplayStatToName = lolDisplayStatToName
    extractLolDisplayStatFromParticipant = extractLolDisplayStatFromParticipant
    lolStatCategoryToName = lolStatCategoryToName

    @Prop({required: true})
    match!: LolMatch
    
    @Prop({required: true})
    currentParticipantId!: number | null | undefined
    participantIdToName: { [id: number] : string | undefined } = {}

    isGroupExpanded: { [g: string]: boolean } = {}

    toggleGroupOpen(g: ELolStatCategory) {
        let key = lolStatCategoryToName(g)
        Vue.set(this.isGroupExpanded, key, !this.isGroupExpanded[key])
    }

    playerName(p: LolParticipant): string {
        if (!(p.participantId in this.participantIdToName)) {
            return p.summonerName
        }
        
        return this.participantIdToName[p.participantId]!
    }

    playerChampionStyle(p: LolParticipant) : any {
        let borderColor: string
        let playerTeam = getTeamIdFromParticipantId(this.match, p.participantId)

        if (!!this.currentParticipantId) {
            let currentTeam = getTeamIdFromParticipantId(this.match, this.currentParticipantId)
            if (p.participantId === this.currentParticipantId) {
                borderColor = 'color-self'
            } else if (currentTeam === playerTeam) {
                borderColor = 'color-friendly'
            } else {
                borderColor = 'color-enemy'
            }
        } else {
            if (playerTeam === 100) {
                borderColor = 'color-blue-team'
            } else {
                borderColor = 'color-red-team'
            }
        }

        return {
            'border-radius': '4px',
            'border': `2px solid var(--${borderColor})`,
        }
    }

    get teamPlayers(): LolParticipant[] {
        return !!this.currentParticipantId ?
            extractSameTeamPlayersFromParticipantId(this.match, this.currentParticipantId)! :
            extractSameTeamPlayersFromTeamId(this.match, 100)
    }

    get enemyPlayers(): LolParticipant[] {
        return !!this.currentParticipantId ?
            extractEnemyTeamPlayersFromParticipantId(this.match, this.currentParticipantId)! :
            extractSameTeamPlayersFromTeamId(this.match, 200)
    }

    get allPlayers(): LolParticipant[] {
        return [
            ...this.teamPlayers,
            ...this.enemyPlayers
        ]
    }

    groups: ELolStatCategory[] = allLolStatCategories
    mounted() {
        for (let g of this.groups) {
            Vue.set(this.isGroupExpanded, lolStatCategoryToName(g), true)
        }
    }
}

</script>

<style scoped>

table {
    border-collapse: collapse !important;
    background-color: #1E1E1E;
    color: #FFFFFF;
}

thead th {
    position: sticky;
    top: 0;
    background-color: #1E1E1E;
    z-index: 1;
}

thead tr {
    background-color: #1E1E1E;
}

tr {
    border-bottom: thin solid rgba(255, 255, 255, 0.12);
}

tr td:first-child {
    padding-left: 20px;
}

.group-row {
    background-color: rgba(255, 255, 255, 0.1);
}

.group-field {
    min-height: 48px;
}

.group-row td {
    padding-left: 4px !important;
}

.stat-field {
    max-width: 150px;
    min-height: 32px;
    margin-top: 2px;
    margin-bottom: 2px;
}

</style>