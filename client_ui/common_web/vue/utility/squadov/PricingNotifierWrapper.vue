<template>
    <div class="d-flex align-center full-width">
        <div class="flex-grow-1">
            <slot v-bind:disabled="disabled"></slot>
        </div>

        <div v-if="disabled">
            <v-tooltip bottom :open-delay="250">
                <template v-slot:activator="{ on, attrs }">
                    <v-btn
                        icon
                        v-on="on"
                        v-bind="attrs"
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

                This feature requires a {{ tier }} tier subscription to SquadOV. Check out SquadOV Pro today!
            </v-tooltip>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { EPricingTier } from '@client/js/squadov/pricing'
import { PricingPageId } from '@client/js/pages'

@Component
export default class PricingNotifierWrapper extends Vue {
    @Prop({required: true})
    tier!: EPricingTier

    get disabled(): boolean {
        return !this.$store.getters.isUserInTier(this.tier)
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