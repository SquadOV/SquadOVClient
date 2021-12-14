<template>
    <div class="d-flex align-center">
        <v-img
            v-if="value !== undefined"
            :src="$root.generateAssetUri(rankImageFName)"
            :max-width="maxWidthHeight"
            :max-height="maxWidthHeight"
            contain
        >
        </v-img>

        <div class="ml-1" v-if="showText">
            {{ rankName }}
        </div>
    </div>
</template>

<script lang="ts">

import { valorantRankToName } from '@client/js/valorant/valorant_utility'
import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'

@Component
export default class ValorantRankTier extends Vue {
    @Prop({type: Number})
    value!: number

    @Prop({type: Number, default: 50})
    maxWidthHeight! : number

    @Prop({type: Boolean, default: false})
    showText!: boolean

    get rankImageFName() : string {
        return `assets/valorant/ranks/TX_CompetitiveTier_Small_${this.value}.png`
    }

    get rankName(): string {
        if (this.value === undefined) {
            return 'Any Rank'
        } else {
            return valorantRankToName(this.value)
        }
    }
}

</script>

<style scoped>

</style>