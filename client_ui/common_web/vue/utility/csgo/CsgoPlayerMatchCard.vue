<template>
    <div v-if="!!player">
        <div class="pa-4 d-flex justify-space-around align-center">
            <span class="text-h5">{{ player.steamAccount.name }}</span>

            <steam-account-display
                :account="player.steamAccount"
            >
            </steam-account-display>
        </div>

        <v-divider></v-divider>
        <v-tabs grow color="primary">
            <v-tab>
                Summary
            </v-tab>

            <v-tab-item>
                <v-container fluid v-if="!!stats">
                    <v-row no-gutters>
                        <v-col cols="6">
                            <generic-stat
                                name="KDA"
                                desc="(Kills + Assists) / Deaths"
                                :value="stats.kda"
                            >
                            </generic-stat>

                            <generic-stat
                                name="HS %"
                                desc="Headshot Percentage"
                                :value="stats.headshots / (stats.headshots + stats.bodyshots + stats.legshots)"
                                percentage
                            >
                            </generic-stat>

                            <generic-stat
                                name="DK"
                                desc="Double Kills"
                                :value="stats.doubleKill"
                                force-integer
                            >
                            </generic-stat>

                            <generic-stat
                                name="QK"
                                desc="Quadra Kills"
                                :value="stats.quadraKill"
                                force-integer
                            >
                            </generic-stat>

                            <generic-stat
                                name="ADR"
                                desc="Average Damage Per Round"
                                :value="stats.adr"
                            >
                            </generic-stat>
                            
                            <generic-stat
                                name="UDR"
                                desc="Utility Damage Per Round"
                                :value="stats.utilityDamage"
                            >
                            </generic-stat>
                        </v-col>

                        <v-col cols="6">
                            <generic-stat
                                name="FB"
                                desc="First Bloods"
                                :value="stats.firstBlood"
                                force-integer
                            >
                            </generic-stat>

                            <generic-stat
                                name="FTD"
                                desc="First To Die"
                                :value="stats.firstToDie"
                                force-integer
                            >
                            </generic-stat>

                            <generic-stat
                                name="TK"
                                desc="Triple Kills"
                                :value="stats.tripleKill"
                                force-integer
                            >
                            </generic-stat>

                            <generic-stat
                                name="PK"
                                desc="Penta Kills"
                                :value="stats.pentaKill"
                                force-integer
                            >
                            </generic-stat>

                            <generic-stat
                                name="MVPs"
                                desc="MVPs"
                                :value="stats.mvps"
                                force-integer
                            >
                            </generic-stat>

                            <generic-stat
                                name="EF"
                                desc="Enemies Flashed"
                                :value="stats.enemiesFlashed"
                                force-integer
                            >
                            </generic-stat>
                        </v-col>
                    </v-row>

                    <v-row no-gutters>
                        <valorant-hit-tracker
                            :headshots="stats.headshots"
                            :bodyshots="stats.bodyshots"
                            :legshots="stats.legshots"
                        >
                        </valorant-hit-tracker>
                    </v-row>
                </v-container>
            </v-tab-item>

            <v-tab>
                Weapons
            </v-tab>

            <v-tab-item>
                <v-data-table
                    :headers="weaponHeaders"
                    :items="weaponItems"
                    hide-default-footer
                >
                    <template v-slot:item.weapon="{ item }">
                        <div class="d-flex justify-space-around align-center">
                            <csgo-weapon-icon
                                :weapon="item.weapon"
                                :height="48"
                            >
                            </csgo-weapon-icon>

                            {{ getCsgoWeaponName(item.weapon) }}
                        </div>
                    </template>

                    <template v-slot:item.kills="{ item }">
                        {{ item.kills }}
                    </template>

                    <template v-slot:item.headshots="{ item }">
                        {{ item.headshots }}
                    </template>
                </v-data-table>
            </v-tab-item>

            <v-tab>
                H2H
            </v-tab>

            <v-tab-item>
                <csgo-head-to-head-display
                    :match="match"
                    :match-user-id="matchUserId"
                    @go-to-event="$emit('go-to-event', arguments[0])"
                >
                </csgo-head-to-head-display>
            </v-tab-item>
        </v-tabs>
    </div>
</template>

<script lang="ts">

import Component, { mixins } from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { CsgoFullMatchDataWrapper } from '@client/js/csgo/match'
import {
    CsgoEventPlayer, CsgoHitgroup, CsgoWeapon, getCsgoWeaponName
} from '@client/js/csgo/events'
import SteamAccountDisplay from '@client/vue/utility/steam/SteamAccountDisplay.vue'
import GenericStat from '@client/vue/utility/GenericStat.vue'
import ValorantHitTracker from '@client/vue/utility/valorant/ValorantHitTracker.vue'
import CsgoWeaponIcon from '@client/vue/utility/csgo/CsgoWeaponIcon.vue'
import CsgoHeadToHeadDisplay from '@client/vue/utility/csgo/CsgoHeadToHeadDisplay.vue'
import CommonComponent from '@client/vue/CommonComponent'

interface CsgoStats {
    kda: number
    headshots: number
    bodyshots: number
    legshots: number
    firstBlood: number
    doubleKill: number
    tripleKill: number
    quadraKill: number
    pentaKill: number
    utilityDamage: number
    mvps: number
    enemiesFlashed: number
    adr: number
    firstToDie: number
}

interface CsgoWeaponStats {
    weapon: CsgoWeapon
    kills: number
    headshots: number
}

@Component({
    components: {
        SteamAccountDisplay,
        GenericStat,
        ValorantHitTracker,
        CsgoWeaponIcon,
        CsgoHeadToHeadDisplay,
    }
})
export default class CsgoPlayerMatchCard extends mixins(CommonComponent) {
    getCsgoWeaponName = getCsgoWeaponName

    @Prop({required: true})
    match!: CsgoFullMatchDataWrapper

    @Prop({required: true})
    matchUserId!: number | null

    get player(): CsgoEventPlayer | undefined {
        if (this.matchUserId === null) {
            return undefined
        }
        return this.match.playerStats(this.matchUserId)
    }

    get stats(): CsgoStats | undefined {
        if (!this.player) {
            return undefined
        }

        let ret: CsgoStats = {
            kda: (this.player.kills + this.player.assists) / this.player.deaths,
            headshots: 0,
            bodyshots: 0,
            legshots: 0,
            firstBlood: 0,
            doubleKill: 0,
            tripleKill: 0,
            quadraKill: 0,
            pentaKill: 0,
            utilityDamage: 0,
            mvps: this.player.mvps,
            enemiesFlashed: 0,
            adr: 0,
            firstToDie: 0,
        }

        let totalRounds = 0
        for (let r of this.match.rounds) {
            let stats = r.userStats(this.player.userId)
            if (!stats) {
                continue
            }

            if (stats.utilityDamage !== null) {
                ret.utilityDamage += stats.utilityDamage
            }

            if (stats.enemiesFlashed !== null) {
                ret.enemiesFlashed += stats.enemiesFlashed
            }

            if (stats.kills == 2) {
                ret.doubleKill += 1
            } else if (stats.kills == 3) {
                ret.tripleKill += 1
            } else if (stats.kills == 4) {
                ret.quadraKill += 1
            } else if (stats.kills >= 5) {
                ret.pentaKill += 1
            }

            for (let d of r._r.damage) {
                if (d.attacker !== this.player.userId) {
                    continue
                }

                if (d.hitgroup == CsgoHitgroup.Head) {
                    ret.headshots += 1
                } else if (d.hitgroup >= CsgoHitgroup.Chest && d.hitgroup <= CsgoHitgroup.RightArm) {
                    ret.bodyshots += 1
                } else if (d.hitgroup >= CsgoHitgroup.LeftLeg) {
                    ret.legshots += 1
                }

                ret.adr += d.damageHealth
            }

            if (r._r.kills.length > 0) {
                let kill = r._r.kills[0]

                if (kill.killer === this.player.userId) {
                    ret.firstBlood += 1
                } else if (kill.victim === this.player.userId) {
                    ret.firstToDie += 1
                }
            }

            totalRounds += 1
        }

        ret.utilityDamage /= totalRounds
        ret.adr /= totalRounds

        return ret
    }

    get weaponHeaders() : any[] {
        let align = 'center'
        return [
            {
                text: 'Weapon',
                value: 'weapon',
                align: align,
            },
            {
                text: 'Kills',
                value: 'kills',
                align: align,
            },
            {
                text: 'Headshots',
                value: 'headshots',
                align: align,
            },
        ]
    }

    get weaponItems() : any[] {
        if (!this.player) {
            return []
        }

        let data: Map<CsgoWeapon, CsgoWeaponStats> = new Map()
        for (let r of this.match.rounds) {
            for (let k of r._r.kills) {
                if (k.killer !== this.player.userId || k.weapon === null) {
                    continue
                }

                if (!data.has(k.weapon)) {
                    data.set(k.weapon, {
                        weapon: k.weapon,
                        kills: 0,
                        headshots: 0,
                    })
                }

                let newStats = data.get(k.weapon)!
                newStats.kills += 1

                if (!!k.headshot) {
                    newStats.headshots += 1
                }
            }
        }
        return Array.from(data.values()).sort((a: CsgoWeaponStats, b: CsgoWeaponStats) => {
            return b.kills - a.kills
        })
    }
}

</script>