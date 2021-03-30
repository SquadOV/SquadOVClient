<template>
    <v-tabs vertical>
        <template v-for="(packet, idx) in deaths">
            <v-tab :key="`tab-${idx}`" :style="deathTabStyle(packet)">
                <div class="d-flex align-center">
                    <div class="text-subtitle-2 font-weight-bold">
                        {{ eventTimeToStr(packet.tm) }}
                    </div>

                    <wow-character-display
                        :character="packet.char"
                    >
                    </wow-character-display>

                    <v-btn
                        outlined
                        fab
                        small
                        color="white"
                        @click.stop="$emit('go-to-time', packet.tm)"
                        v-if="hasVod"
                    >
                        <v-icon>mdi-play</v-icon>
                    </v-btn>
                </div>
            </v-tab>

            <v-tab-item :key="`item-${idx}`">
                <wow-death-recap
                    :match-uuid="matchUuid"
                    :user-id="userId"
                    :character="packet.char"
                    :match-characters="matchCharacters"
                    :death="packet.death"
                    :start-time="startTime"
                    @go-to-time="$emit('go-to-time', arguments[0])"
                >
                </wow-death-recap>
            </v-tab-item>
        </template>
    </v-tabs>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { UnifiedWowEventContainer, WowDeath } from '@client/js/wow/events'
import { WowCharacter } from '@client/js/wow/character'
import { secondsToTimeString } from '@client/js/time'
import { getGenericWinColor, getGenericLossColor, colorToCssString } from '@client/js/color'
import WowCharacterDisplay from '@client/vue/utility/wow/WowCharacterDisplay.vue'
import WowDeathRecap from '@client/vue/utility/wow/WowDeathRecap.vue'

interface DeathPacket {
    tm: Date
    death: WowDeath
    char: WowCharacter
}

@Component({
    components: {
        WowCharacterDisplay,
        WowDeathRecap
    }
})
export default class WowDeathRecapAnalysis extends Vue {
    @Prop({required: true})
    matchUuid!: string

    @Prop({required: true})
    userId!: number

    @Prop({required: true})    
    startTime!: Date

    @Prop({required: true})
    matchCharacters!: WowCharacter[]

    @Prop({required: true})
    unifiedEvents!: UnifiedWowEventContainer[]

    @Prop({type: Boolean, required: true})
    hasVod!: boolean

    @Prop({default: 0})
    friendlyTeam!: number

    eventTimeToStr(tm : Date): string {
        let secs = (tm.getTime() - this.startTime.getTime()) / 1000
        return secondsToTimeString(secs)
    }

    get guidToChar(): Map<string, WowCharacter> {
        let ret = new Map()
        for (let c of this.matchCharacters) {
            ret.set(c.guid, c)
        }
        return ret
    }

    get deaths(): DeathPacket[] {
        return this.unifiedEvents.filter((ele: UnifiedWowEventContainer) => !!ele.death).map((ele: UnifiedWowEventContainer) => {
            return <DeathPacket>{
                tm: ele.tm,
                death: ele.death!,
                char: this.guidToChar.get(ele.death!.guid)!
            }
        }).filter((ele: DeathPacket) => !!ele.char)
    }

    deathTabStyle(d: DeathPacket): any {
        return {
            'border-right': `2px solid ${colorToCssString(d.char.team === this.friendlyTeam ? getGenericWinColor() : getGenericLossColor())}`
        }
    }
}

</script>