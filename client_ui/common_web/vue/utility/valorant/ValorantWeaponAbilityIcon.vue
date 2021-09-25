<template>
    <v-img
        :src="$root.generateAssetUri(equipFname)"
        :max-height="maxHeight"
        :max-width="maxWidth"
        :style="style"
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
    equipType! : string

    @Prop({required: true})
    equipId! : string

    @Prop({type: Number, default: 50})
    maxHeight!: number

    @Prop({type: Number, default: 50})
    maxWidth!: number

    @Prop({type: Boolean, default: false})
    reverseWeapon!: boolean

    get agentName() : string {
        let cnt = getValorantContent(null)
        return cnt.agentIdToFilesystemName(this.agent)
    }

    get isReversed(): boolean {
        return this.equipType == 'Weapon' && this.reverseWeapon
    }

    get style(): any {
        let ret: any = {}
        if (this.isReversed) {
            ret['transform'] = 'scaleX(-1)'
        }
        return ret
    }

    get equipFname() : string {
        if (this.equipType == 'Weapon') {
            let cnt = getValorantContent(null)
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