<template>
    <v-sheet
    >
        <div class="pa-4 text-center" :style="headerDivStyle">
            <span class="text-overline font-weight-bold">{{ headerText }}</span>
        </div>
        <v-divider></v-divider>
        <v-data-table
            :headers="headers"
            :items="items"
            hide-default-footer
        >
            <template v-slot:item.player="{ item }">
                <div :class="`d-flex justify-space-around align-center ${ mirror ? 'flex-row-reverse' : ''}`">
                    <valorant-agent-icon
                        :agent="item.player._p.agentId"
                        :patch="match._details.patchId"
                        :width-height="40"
                        circular
                    >
                    </valorant-agent-icon>

                    <span>{{ agentName(item.player._p.agentId) }}</span>
                </div>
            </template>

            <template v-slot:item.kda="{ item }">
                {{ item.kda.kills }} / {{ item.kda.deaths }} / {{ item.kda.assists }}
            </template>

            <template v-slot:item.score="{ item }">
                {{ item.score }}
            </template>

            <template v-slot:item.damage="{ item }">
                {{ item.damage }}
            </template>

            <template v-slot:item.inventory="{ item }">
                <div :class="`d-flex justify-space-around align-center ${ mirror ? 'flex-row-reverse' : ''}`">
                    <valorant-weapon-ability-icon
                        :agent="item.player._p.agentId"
                        :patch="match._details.patchId"
                        equip-type="Weapon"
                        :equip-id="item.loadout.weapon"
                        :max-height="40"
                        :max-width="150"
                    >
                    </valorant-weapon-ability-icon>

                    <valorant-armor-icon
                        :equip-id="item.loadout.armor"
                        :max-width-height="40"
                    >
                    </valorant-armor-icon>
                </div>
            </template>
        </v-data-table>
    </v-sheet>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import {
    ValorantMatchDetailsWrapper,
    ValorantMatchPlayerWrapper,
    ValorantMatchTeamWrapper,
    ValorantMatchRoundWrapper
} from '@client/js/valorant/valorant_matches_parsed'
import {
    getBlueTeamColor,
    getRedTeamColor,
    getSameTeamColor
} from '@client/js/valorant/valorant_colors'
import { Color } from '@client/js/color'
import { getValorantContent } from '@client/js/valorant/valorant_content'

import ValorantAgentIcon from '@client/vue/utility/valorant/ValorantAgentIcon.vue'
import ValorantWeaponAbilityIcon from '@client/vue/utility/valorant/ValorantWeaponAbilityIcon.vue'
import ValorantArmorIcon from '@client/vue/utility/valorant/ValorantArmorIcon.vue'

@Component({
    components: {
        ValorantAgentIcon,
        ValorantWeaponAbilityIcon,
        ValorantArmorIcon
    }
})
export default class ValorantTeamRoundDisplay extends Vue {
    @Prop({required: true})
    match!: ValorantMatchDetailsWrapper

    @Prop({required: true})
    round!: ValorantMatchRoundWrapper

    @Prop({required: true})
    team! : ValorantMatchTeamWrapper

    @Prop()
    currentPlayer! : ValorantMatchPlayerWrapper | null

    @Prop({type: Boolean, default: false })
    mirror! : boolean

    agentName(id : string) : string {
        let cnt = getValorantContent(this.match._details.patchId)
        return cnt.agentIdToName(id)
    }

    get headers() : any[] {
        let align = 'center'
        let hdr = [
            {
                text: 'Player',
                value: 'player',
                align: align,
                sort: (a : any, b : any) : number => {
                    let aName = this.agentName(a._p.agentId) 
                    let bName = this.agentName(b._p.agentId)
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
                text: 'Score',
                value: 'score',
                align: align,
            },
            {
                text: 'Damage',
                value: 'damage',
                align: align,
            },
            {
                text: 'Inventory',
                value: 'inventory',
                align: align,
            },
        ]

        if (this.mirror) {
            hdr.reverse()
        }

        return hdr
    }

    get items() : any[] {
        return this.team.getPlayersDescendingCS().map((ele : ValorantMatchPlayerWrapper) => {
            return {
                player: ele,
                kda: {
                    kills: this.round.getKillsForPlayer(ele._p.puuid),
                    deaths: this.round.getDeathsForPlayer(ele._p.puuid),
                    assists: this.round.getAssistsForPlayer(ele._p.puuid),
                },
                score: this.round.getCombatScoreForPlayer(ele._p.puuid),
                damage: this.round.getDamageDoneByPlayer(ele._p.puuid),
                loadout: this.round.getLoadoutForPlayer(ele._p.puuid)
            }
        })
    }

    get headerText() : string {
        if (!!this.currentPlayer) {
            if (this.currentPlayer._p.teamId == this.team._t.teamId) {
                return 'Your Team'
            } else {
                return 'Enemy Team'
            }
        } else {
            return `${this.team._t.teamId} Team`
        }
    }

    get headerDivStyle() : any {
        let color : Color = { r : 0, g : 0, b : 0}
        if (!!this.currentPlayer) {
            if (this.currentPlayer._p.teamId == this.team._t.teamId) {
                color = getSameTeamColor()
            } else {
                color = getRedTeamColor()
            }
        } else {
            if (this.team._t.teamId == 'Blue') {
                color = getBlueTeamColor()
            } else {
                color = getRedTeamColor()
            }
        }

        return {
            'background-color': `rgba(${color.r}, ${color.g}, ${color.b}, 0.5)`
        }
    }
}

</script>

<style scoped>

</style>