<template>
    <v-data-table
        :headers="headers"
        :items="items"
        hide-default-footer
    >
        <template v-slot:item.equipment="{ item }">
            <div class="d-flex justify-space-around align-center">
                <valorant-weapon-ability-icon
                    :agent="agent"
                    :max-height="50"
                    :max-width="120"
                    :equip-type="item.equipment.type"
                    :equip-id="item.equipment.id"
                >
                </valorant-weapon-ability-icon>    

                <span>{{getEquipmentName(item.equipment.type, item.equipment.id)}}</span>
            </div>
        </template>

        <template v-slot:item.kills="{ item }">
            {{ item.kills }}
        </template>

        <template v-slot:item.altFire="{ item }">
            {{ item.altFire }}
        </template>
    </v-data-table>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import {
    ValorantMatchKillWrapper,
} from '@client/js/valorant/valorant_matches_parsed'
import ValorantWeaponAbilityIcon from '@client/vue/utility/valorant/ValorantWeaponAbilityIcon.vue'
import { getValorantContent, AbilityKey } from '@client/js/valorant/valorant_content'
import { group } from 'console'

@Component({
    components: {
        ValorantWeaponAbilityIcon
    }
})
export default class ValorantWeaponAbilityKillStats extends Vue {
    @Prop({required: true})
    agent! : string

    @Prop({type:Array, required: true})
    kills! : ValorantMatchKillWrapper[]

    getEquipmentName(typ : string, id : string) : string {
        let cnt = getValorantContent(null)
        if (typ == 'Weapon') {
            return cnt.weaponIdToName(id)
        } else if (typ == 'Ability') {
            return cnt.abilityIdToName(this.agent, <AbilityKey>id)
        }
        return ''
    }

    get headers() : any[] {
        let align = 'center'
        return [
            {
                text: 'Equipment',
                value: 'equipment',
                align: align,
            },
            {
                text: 'Kills',
                value: 'kills',
                align: align,
            },
            {
                text: 'Alt Fire',
                value: 'altFire',
                align: align,
            }
        ]
    }

    get items() : any[] {
        // Identify by weapon type since this should just be using 1 agent so the ability
        // names won't overlap.
        let groupedKills = new Map<string, ValorantMatchKillWrapper[]>()

        this.kills.forEach((ele :ValorantMatchKillWrapper) => {
            let key = ele._k.finishingDamage.damageItem
            if (!groupedKills.has(key)) {
                groupedKills.set(key, [])
            }
            
            groupedKills.get(key)!.push(ele)
        })

        let ret : any[] = []
        groupedKills.forEach((kills : ValorantMatchKillWrapper[], item : string) => {
            ret.push({
                equipment: {
                    type: kills[0]._k.finishingDamage.damageType,
                    id: item,
                },
                kills: kills.length,
                altFire: kills.reduce((acc : number, k : ValorantMatchKillWrapper) => {
                    return acc + (k._k.finishingDamage.isSecondaryFireMode ? 1 : 0)
                }, 0)
            })
        })

        return ret.sort((a : any, b : any) => {
            return b.kills - a.kills
        })
    }
}

</script>