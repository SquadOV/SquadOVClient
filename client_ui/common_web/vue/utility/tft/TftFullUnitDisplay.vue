<template>
    <div class="d-flex justify-center flex-wrap">
        <tft-single-unit-display
            class="mx-2"
            v-for="(u, idx) in sortedUnits"
            :key="idx"
            :unit="u"
            :tft-set="tftSet"
            :width="singleWidth"
        >
        </tft-single-unit-display>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { TftUnit } from '@client/js/tft/units'
import TftSingleUnitDisplay from '@client/vue/utility/tft/TftSingleUnitDisplay.vue'

@Component({
    components: {
        TftSingleUnitDisplay
    }
})
export default class TftFullUnitDisplay extends Vue {
    @Prop({type: Array, required: true})
    units!: TftUnit[]

    @Prop({required: true})
    tftSet!: string

    @Prop({default: 64})
    singleWidth!: number

    get sortedUnits(): TftUnit[] {
        let units = [...this.units]
        units.sort((a: TftUnit, b: TftUnit) => {
            return b.tier - a.tier || b.rarity - a.rarity
        })
        return units
    }
}

</script>