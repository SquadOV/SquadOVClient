<template>
    <div>
        <v-sheet
            rounded
            :elevation="highlight ? 10: 2"
            :class="`d-flex flex-column justify-center align-center pa-4 ${highlight ? 'highlight-pricing' : ''}`"
        >
            <div class="text-h6 font-weight-bold">
                {{ priceTier }}
            </div>

            <div>
                ${{ pricePerMonth }} USD per month
            </div>
        
            <v-divider class="mt-4" style="width: 80%"></v-divider>

            <v-list width="100%">
                <v-list-item
                    v-for="(b, idx) in benefits"
                    :key="`benefit-${idx}`"
                >
                    <v-list-item-avatar>
                        <v-icon :color="highlight ? 'success' : undefined">mdi-check-circle</v-icon>
                    </v-list-item-avatar>

                    <v-list-item-content>
                        <v-list-item-title class="d-flex align-center" style="word-wrap: break-word; white-space: pre-line;">
                            <span>{{ b }}</span>
                        </v-list-item-title>
                    </v-list-item-content>
                </v-list-item>
            </v-list>
        </v-sheet>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { EPricingTier } from '@client/js/squadov/pricing'

@Component
export default class PricingTier extends Vue {
    @Prop({required: true})
    tier!: EPricingTier

    @Prop({type: Boolean, default: false})
    highlight!: boolean

    get pricePerMonth(): string {
        switch (this.tier) {
            case EPricingTier.Basic:
                return '0'
            case EPricingTier.Silver:
                return '5'
            case EPricingTier.Gold:
                return '10'
            case EPricingTier.Diamond:
                return '15'
        }
        return ''
    }

    get priceTier(): string {
        switch (this.tier) {
            case EPricingTier.Basic:
                return 'Basic'
            case EPricingTier.Silver:
                return 'Silver'
            case EPricingTier.Gold:
                return 'Gold'
            case EPricingTier.Diamond:
                return 'Diamond'
        }
        return ''
    }

    get benefits(): string[] {
        switch (this.tier) {
            case EPricingTier.Basic:
                return [
                    '720p 60fps Recordings',
                    'No Ads',
                    'Advanced Search and Organization',
                    'Unlimited Clips',
                ]
            case EPricingTier.Silver:
                return [
                    'Everything in Basic',
                    '1080p 60fps Recordings',
                    'Unlimited Storage',
                    'No Watermarks',
                ]
            case EPricingTier.Gold:
                return [
                    'Everything in Silver',
                    '1440p 60fps Recordings',
                    'Unlimited Squad Sizes',
                    'Priority Support',
                    'Early Access'
                ]
            case EPricingTier.Diamond:
                return [
                    'Everything in Gold',
                    'Native 144fps Recordings',
                    'High VOD Processing Priority',
                    'More Coming Soon!'
                ]
        }
        return []
    }
}

</script>

<style scoped>

.highlight-pricing {
    border: 2px #4caf50 solid;
}

</style>