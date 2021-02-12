<template>
    <div class="d-flex justify-center flex-wrap">
        <tft-single-trait-display
            v-for="(trait, idx) in filteredTraits"
            :key="idx"
            :trait="trait"
            :tft-set="tftSet"
            :width-height="singleWidthHeight"
        >
        </tft-single-trait-display>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { TftTrait } from '@client/js/tft/traits'
import TftSingleTraitDisplay from '@client/vue/utility/tft/TftSingleTraitDisplay.vue'

@Component({
    components: {
        TftSingleTraitDisplay
    }
})
export default class TftFullTraitDisplay extends Vue {
    @Prop({type: Array, required: true})
    traits!: TftTrait[]

    @Prop({required: true})
    tftSet!: string

    @Prop({default: 32})
    singleWidthHeight!: number

    get filteredTraits(): TftTrait[] {
        let filtered = this.traits.filter((ele: TftTrait) => {
            return ele.style >= 1
        })

        filtered.sort((a: TftTrait, b: TftTrait) => {
            return b.style - a.style
        })

        return filtered
    }
}

</script>