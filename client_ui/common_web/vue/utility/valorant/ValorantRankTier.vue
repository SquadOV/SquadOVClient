<template>
    <v-img
        :src="$root.generateAssetUri(rankImageFName)"
        :max-width="maxWidthHeight"
        :max-height="maxWidthHeight"
        contain
    >
    </v-img>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'

@Component
export default class ValorantRankTier extends Vue {
    @Prop({type: Number})
    value!: number

    @Prop({type: Number, default: 50})
    maxWidthHeight! : number

    get rankImageFName() : string {
        return `assets/valorant/ranks/TX_CompetitiveTier_Small_${this.value}.png`
    }
}

</script>

<style scoped>

</style>