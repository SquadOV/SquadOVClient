<template>
    <v-img
        :src="equipFname"
        :max-height="maxHeight"
        :max-width="maxWidth"
        contain
    >
    </v-img>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { getValorantContent } from '@client/js/valorant/valorant_content'

@Component
export default class ValorantWeaponAbilityIcon extends Vue {
    @Prop({required: true})
    agent! : string

    @Prop({required: true})
    patch! : string

    @Prop({required: true})
    equipType! : string

    @Prop({required: true})
    equipId! : string

    @Prop({type: Number, default: 50})
    maxHeight!: number

    @Prop({type: Number, default: 50})
    maxWidth!: number

    get agentName() : string {
        let cnt = getValorantContent(this.patch)
        return cnt.agentIdToName(this.agent)
    }

    get equipFname() : string {
        if (this.equipType == 'Weapon') {
            let cnt = getValorantContent(this.patch)
            let weapon = cnt.weaponIdToName(this.equipId)
            return `assets/valorant/weapons/${weapon}.png`
        } else if (this.equipType == 'Ability') {
            return `assets/valorant/agents/${this.agentName}/${this.equipId}.png`
        } else if (this.equipType == 'Bomb') {
            return 'assets/valorant/bomb/explode.png'
        }
        return ''
    }
}

</script>

<style scoped>

</style>