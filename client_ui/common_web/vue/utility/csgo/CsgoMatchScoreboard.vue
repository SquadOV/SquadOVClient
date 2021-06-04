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
                <tr :style="teamRowStyle(item.userId)" @click="$emit('update:matchUserId', item.userId)">
                    <td class="text-center">
                        <div class="d-flex justify-space-around align-center">
                            <steam-account-display
                                :width="48"
                                :height="48"
                                :account="item.account"
                            >
                            </steam-account-display>

                            <span v-if="!!item.account">{{ item.account.name }}</span>
                        </div>                    
                    </td>

                    <td class="text-center">
                        {{ item.kda.kills }} / {{ item.kda.deaths }} / {{ item.kda.assists }}
                    </td>

                    <td class="text-center">
                        {{ item.adr.toFixed(2) }}
                    </td>

                    <td class="text-center">
                        {{ item.mvps }}
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
import { CsgoFullMatchDataWrapper } from '@client/js/csgo/match'
import { SteamAccount } from '@client/js/steam/account'
import { ColorA, colorFromElementTheme } from '@client/js/color'
import SteamAccountDisplay from '@client/vue/utility/steam/SteamAccountDisplay.vue'

interface TableData {
    userId: number
    kda: {
        kills: number
        deaths: number
        assists: number
    }
    account: SteamAccount | undefined
    adr: number
    mvps: number
}

@Component({
    components: {
        SteamAccountDisplay,
    }
})
export default class CsgoMatchScoreboard extends Vue {
    @Prop({required: true})
    match!: CsgoFullMatchDataWrapper

    @Prop({required: true})
    matchUserId!: number | null

    get headers() : any[] {
        let align = 'center'
        let hdr = [
            {
                text: 'Player',
                value: 'account',
                align: align,
                sort: (a : SteamAccount | undefined, b : SteamAccount | undefined) : number => {
                    let aName = a?.name
                    let bName = b?.name

                    if (!!aName && !!bName) {
                        if (aName < bName) {
                            return -1
                        } else if (aName > bName) {
                            return 1
                        } else {
                            return 0
                        }
                    } else if (!!aName) {
                        return -1
                    } else if (!!bName) {
                        return 1
                    } else {
                        return 0
                    }
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
                text: 'ADR',
                value: 'adr',
                align: align,
            },
            {
                text: 'MVPs',
                value: 'mvp',
                align: align,
            },
        ]

        return hdr
    }

    get items() : TableData[] {
        return this.match.kdaOrderedPlayers().map((uid: number) => {
            let stats = this.match.playerStats(uid)
            let data: TableData = {
                userId: uid,
                kda: {
                    kills: 0,
                    deaths: 0,
                    assists: 0,
                },
                account: undefined,
                adr: 0,
                mvps: 0,
            }

            if (!!stats) {
                data.kda.kills = stats.kills
                data.kda.deaths = stats.deaths
                data.kda.assists = stats.assists
                data.account = stats.steamAccount
                data.adr = this.match.adr(uid)
                data.mvps = stats.mvps
            }
            return data
        })
    }

    teamRowStyle(uid: number): any {
        let style : any = {}

        if (this.matchUserId === uid) {
            style['border-left'] = '5px solid var(--color-self)'
        }

        if (this.matchUserId !== null) {
            let color : ColorA = { r : 0, g : 0, b : 0, a: 0 }
            let lastRound = this.match.rounds[this.match.rounds.length - 1]
            if (!!lastRound) {
                let refTeam = lastRound.userTeam(this.matchUserId)
                let testTeam = lastRound.userTeam(uid)

                if (refTeam == testTeam) {
                    color = colorFromElementTheme(this.$parent.$el, 'color-top-place')
                } else {
                    color = colorFromElementTheme(this.$parent.$el, 'color-bottom-place')
                }
                style['background-color'] = `rgba(${color.r}, ${color.g}, ${color.b}, 0.5)`
            }
        }
        
        return style
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