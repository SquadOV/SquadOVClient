<template>
    <div class="d-flex justify-space-around">
        <div class="d-flex justify-center align-end" v-if="!!subject">
            <lol-participant-display
                :participant="subject.primary"
                :match="match"
                :width-height="height"
                :current-participant-id="currentParticipantId"
            >
            </lol-participant-display>
        </div>

        <div class="d-flex justify-center align-center" v-if="verb !== null">
            <v-icon v-if="verb == Verb.EKill" small>
                mdi-sword-cross
            </v-icon>

            <v-icon v-else-if="verb == Verb.EPlace" small>
                mdi-map-marker
            </v-icon>
        </div>

        <div class="d-flex justify-center align-center" v-if="!!target">
            <lol-participant-display
                v-if="!!target.participant"
                :participant="target.participant"
                :match="match"
                :width-height="height"
                :current-participant-id="currentParticipantId"
            >
            </lol-participant-display>

            <lol-static-target-display
                v-if="target.other !== undefined"
                :target="target.other"
                :height="height"
            >
            </lol-static-target-display>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { LolMatch, getPlayerFromParticipantId } from '@client/js/lol/matches'
import { LolMatchTimeline, LolMatchEvent, LolStaticTargets } from '@client/js/lol/timeline'
import { LolParticipant } from '@client/js/lol/participant'

import LolParticipantDisplay from '@client/vue/utility/lol/LolParticipantDisplay.vue'
import LolStaticTargetDisplay from '@client/vue/utility/lol/LolStaticTargetDisplay.vue'

interface Subject {
    primary: LolParticipant
    assistants: LolParticipant[]
}

enum Verb {
    EKill,
    EPlace
}

interface Target {
    participant?: LolParticipant | undefined
    other?: LolStaticTargets
}

@Component({
    components: {
        LolParticipantDisplay,
        LolStaticTargetDisplay,
    }
})
export default class LolEventDisplay extends Vue {
    Verb = Verb
    LolStaticTargets = LolStaticTargets

    @Prop({required: true})
    match!: LolMatch

    @Prop({required: true})
    timeline!: LolMatchTimeline

    @Prop({required: true})
    inputEvent!: LolMatchEvent

    @Prop({required: true})
    currentParticipantId!: number | null | undefined

    @Prop({type: Boolean, default: false})
    mini!: boolean

    @Prop({default: 48})
    height!: number

    get subject(): Subject | null {
        if (this.mini) {
            return null
        }

        switch (this.inputEvent.type) {
            case 'CHAMPION_KILL':
            case 'WARD_KILL':
            case 'BUILDING_KILL':
            case 'ELITE_MONSTER_KILL': {
                let primary = getPlayerFromParticipantId(this.match, this.inputEvent.killerId!)
                if (!primary) {
                    return null
                }

                return {
                    primary,
                    assistants: <LolParticipant[]>(!!this.inputEvent.assistingParticipantIds ?
                        this.inputEvent.assistingParticipantIds.map((ele: number) => {
                            return getPlayerFromParticipantId(this.match, ele)
                        }).filter((ele: LolParticipant | undefined) => {
                            return !!ele
                        }) : [])
                }
            }
            case 'WARD_PLACED': {
                let primary = getPlayerFromParticipantId(this.match, this.inputEvent.creatorId!)
                if (!primary) {
                    return null
                }

                return {
                    primary,
                    assistants: []
                }
            }
        }
        return null
    }

    get verb(): Verb | null {
        if (this.mini && this.inputEvent.type !== 'CHAMPION_KILL') {
            return null
        }

        switch (this.inputEvent.type) {
            case 'CHAMPION_KILL':
            case 'WARD_KILL':
            case 'BUILDING_KILL':
            case 'ELITE_MONSTER_KILL':
                return Verb.EKill
            case 'WARD_PLACED':
                return Verb.EPlace
        }
        return null
    }

    get target(): Target | null {
        switch (this.inputEvent.type) {
            case 'CHAMPION_KILL':
                if (this.mini) {
                    return null
                }
                return {
                    participant: getPlayerFromParticipantId(this.match, this.inputEvent.victimId!)
                }
            case 'WARD_KILL':
            case 'WARD_PLACED':
                if (this.inputEvent.wardType === 'SIGHT_WARD') {
                    return {
                        other: LolStaticTargets.ESightWard
                    }
                } else if (this.inputEvent.wardType === 'CONTROL_WARD') {
                    return {
                        other: LolStaticTargets.EControlWard
                    }
                } else if (this.inputEvent.wardType === 'YELLOW_TRINKET') {
                    return {
                        other: LolStaticTargets.EYellowTrinket
                    }
                } else if (this.inputEvent.wardType === 'BLUE_TRINKET') {
                    return {
                        other: LolStaticTargets.EBlueTrinket
                    }
                } else {
                    return {
                        other: LolStaticTargets.EWard
                    }
                }
            case 'BUILDING_KILL':
                if (this.inputEvent.buildingType === 'TOWER_BUILDING') {
                    return {
                        other: LolStaticTargets.ETower
                    }
                } else if (this.inputEvent.buildingType === 'INHIBITOR_BUILDING') {
                    return {
                        other: LolStaticTargets.EInhibitor
                    }
                }
            case 'ELITE_MONSTER_KILL':
                if (this.inputEvent.monsterType === 'DRAGON') {
                    if (this.inputEvent.monsterSubType === 'AIR_DRAGON') {
                        return {
                            other: LolStaticTargets.EDragonAir
                        }
                    } else if (this.inputEvent.monsterSubType === 'FIRE_DRAGON') {
                        return {
                            other: LolStaticTargets.EDragonFire
                        }
                    } else if (this.inputEvent.monsterSubType === 'EARTH_DRAGON') {
                        return {
                            other: LolStaticTargets.EDragonEarth
                        }
                    } else if (this.inputEvent.monsterSubType === 'WATER_DRAGON') {
                        return {
                            other: LolStaticTargets.EDragonWater
                        }
                    } else if (this.inputEvent.monsterSubType === 'ELDER_DRAGON') {
                        return {
                            other: LolStaticTargets.EDragonElder
                        }
                    } else {
                        return {
                            other: LolStaticTargets.EDragon
                        }
                    }
                } else if (this.inputEvent.monsterType === 'BARON_NASHOR') {
                    return {
                        other: LolStaticTargets.EBaron
                    }
                } else if (this.inputEvent.monsterType === 'RIFTHERALD') {
                    return {
                        other: LolStaticTargets.ERiftHerald
                    }
                }
        }
        return null
    }
}


</script>