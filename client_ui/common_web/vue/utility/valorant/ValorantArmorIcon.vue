<template>
    <v-img
        v-if="equipFname != ''"
        :src="$root.generateAssetUri(equipFname)"
        :max-height="maxWidthHeight"
        :max-width="maxWidthHeight"
        contain
    >
    </v-img>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'

@Component
export default class ValorantWeaponAbilityIcon extends Vue {
    @Prop({required: true})
    equipId! : string | null

    @Prop({type: Number, default: 50})
    maxWidthHeight!: number

    get equipFname() : string {
        if (this.equipId == '822BCAB2-40A2-324E-C137-E09195AD7692') {
            return 'assets/valorant/armor/heavy.png'
        } else if (this.equipId == '4DEC83D5-4902-9AB3-BED6-A7A390761157') {
            return 'assets/valorant/armor/light.png'
        }
        return ''
    }
}

</script>

<style scoped>

</style>