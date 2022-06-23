<template>
    <v-btn
        icon
        :to="pricingTo"
        style="pointer-events: auto !important;"
    >
        <v-icon
            :style="iconStyle"
        >
            mdi-star-circle
        </v-icon>
    </v-btn>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { EPricingTier } from '@client/js/squadov/pricing'
import { PricingPageId } from '@client/js/pages'

@Component
export default class SubIcon extends Vue {
    @Prop({required: true})
    tier!: EPricingTier

    @Prop({type: Boolean, default: false})
    hideBasic!: boolean

    get visible(): boolean {
        return !this.hideBasic || this.tier !== EPricingTier.Basic
    }

    get iconStyle(): any {
        return {
            color: (this.tier == EPricingTier.Gold) ? '#FFFF00' :
                (this.tier == EPricingTier.Diamond) ? '#E040FB' :  '#E0E0E0'
        }
    }

    get pricingTo(): any {
        return {
            name: PricingPageId
        }
    }
}

</script>