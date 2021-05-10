<template>
    <v-sheet
    >
        <div :class="`pa-4 d-flex ${ mirror ? 'flex-row-reverse' : ''} align-center justify-center`" :style="headerDivStyle">
            <csgo-team-icon
                class="mx-4"
                :team="team"
                :width-height="48"
            >
            </csgo-team-icon>
            <div class="text-overline font-weight-bold">{{ headerText }}</div>
        </div>
        <v-divider></v-divider>
        <v-data-table
            :headers="headers"
            :items="items"
            hide-default-footer
        >
            <template v-slot:item.player="{ item }">
                <div :class="`d-flex justify-space-around align-center ${ mirror ? 'flex-row-reverse' : ''}`">
                    <steam-account-display
                        :width="48"
                        :height="48"
                        :account="item.account"
                    >
                    </steam-account-display>

                    <span v-if="!!item.account">{{ item.account.name }}</span>
                </div>
            </template>

            <template v-slot:item.kda="{ item }">
                {{ item.kills }} / {{ item.deaths }} / {{ item.assists }}
            </template>

            <template v-slot:item.damage="{ item }">
                {{ item.damage.toLocaleString() }} ({{ item.utilityDamage.toLocaleString() }})
            </template>

            <template v-slot:item.equipment="{ item }">
                <csgo-loadout-display
                    :weapons="item.weapons"
                    :has-defuse="item.hasDefuse"
                    :has-helmet="item.hasHelmet"
                    :armor="item.armor"
                >
                </csgo-loadout-display>
            </template>

            <template v-slot:item.money="{ item }">
                ${{ item.money.toLocaleString() }}
            </template>
        </v-data-table>
    </v-sheet>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { CsgoFullMatchDataWrapper, CsgoEventRoundWrapper} from '@client/js/csgo/match'
import { Color, getGenericWinColor, getGenericLossColor, colorToCssString } from '@client/js/color'
import { CsgoTeam, CsgoWeapon } from '@client/js/csgo/events'
import { SteamAccount } from '@client/js/steam/account'
import CsgoTeamIcon from '@client/vue/utility/csgo/CsgoTeamIcon.vue'
import SteamAccountDisplay from '@client/vue/utility/steam/SteamAccountDisplay.vue'
import CsgoLoadoutDisplay from '@client/vue/utility/csgo/CsgoLoadoutDisplay.vue'

interface TableData {
    kills: number
    deaths: number
    assists: number
    account: SteamAccount | undefined
    money: number
    equipValue: number
    damage: number
    utilityDamage: number
    weapons: CsgoWeapon[]
    hasDefuse: boolean
    hasHelmet: boolean
    armor: number
}

@Component({
    components: {
        CsgoTeamIcon,
        SteamAccountDisplay,
        CsgoLoadoutDisplay,
    }
})
export default class CsgoTeamRoundScoreboard extends Vue {
    @Prop({required: true})
    match!: CsgoFullMatchDataWrapper

    @Prop({required: true})
    currentRound!: number

    @Prop({required: true})
    matchUserId!: number | null

    @Prop({required: true})
    team!: CsgoTeam

    @Prop({type: Boolean, default: false })
    mirror! : boolean

    get userTeam(): CsgoTeam {
        if (this.matchUserId === null || !this.roundData) {
            return CsgoTeam.Spectate
        }
        return this.roundData.userTeam(this.matchUserId)
    }

    get roundData(): CsgoEventRoundWrapper | undefined {
        return this.match.round(this.currentRound)
    }

    get headers() : any[] {
        let align = 'center'
        let hdr = [
            {
                text: 'Player',
                value: 'player',
                align: align,
            },
            {
                text: 'KDA',
                value: 'kda',
                align: align,
            },
            {
                text: 'Damage (Utility)',
                value: 'damage',
                align: align,
            },
            {
                text: 'Equipment',
                value: 'equipment',
                align: align,
            },
            {
                text: 'Money',
                value: 'money',
                align: align,
            }
        ]

        if (this.mirror) {
            hdr.reverse()
        }
        return hdr
    }

    get items() : any[] {
        if (!this.roundData) {
            return []
        }
        return this.roundData.orderedTeamPlayers(this.team).map((ele: number) => {
            let stats = this.roundData!.userStats(ele)
            let base: TableData = {
                kills: 0,
                deaths: 0,
                assists: 0,
                account: undefined,
                money: 0,
                equipValue: 0,
                damage: 0,
                utilityDamage: 0,
                weapons: [],
                hasDefuse: false,
                hasHelmet: false,
                armor: 0,
            }
            
            if (!!stats) {
                base.kills = stats.kills
                base.deaths = stats.deaths
                base.assists = stats.assists
                base.account = this.match.steamAccount(this.match.userIdToSteamId(stats.userId))

                if (!!stats.money) {
                    base.money = stats.money
                }

                if (!!stats.equipmentValue) {
                    base.equipValue = stats.equipmentValue
                }

                base.weapons = stats.weapons
                
                if (!!stats.damage) {
                    base.damage = stats.damage
                }

                if (!!stats.utilityDamage) {
                    base.utilityDamage = stats.utilityDamage
                }

                if (!!stats.hasDefuse) {
                    base.hasDefuse = stats.hasDefuse
                }

                if (!!stats.hasHelmet) {
                    base.hasHelmet = stats.hasHelmet
                }

                if (!!stats.armor) {
                    base.armor = stats.armor
                }
            }

            return base
        })
    }

    get headerText(): string {
        if (this.team == this.userTeam) {
            return 'Your Team'
        } else {
            return 'Enemy Team'
        }
    }

    get headerDivStyle() : any {
        let color : Color = { r : 0, g : 0, b : 0}
        if (this.team == this.userTeam) {
            color = getGenericWinColor()
        } else {
            color = getGenericLossColor()
        }

        return {
            'background-color': `${colorToCssString(color)}`
        }
    }
}

</script>