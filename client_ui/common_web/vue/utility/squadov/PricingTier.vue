<template>
    <div class="d-flex flex-column align-center">
        <div
            class="recommended-highlight font-weight-bold text-uppercase"
            v-if="highlight"
        >
            🎉<span class="mx-2">Recommended</span> 🎉
        </div>

        <v-sheet
            rounded
            :elevation="highlight ? 10: 2"
            :class="`d-flex flex-column justify-center align-center pa-4 ${highlight ? 'highlight-pricing' : ''}`"
        >
            <div class="text-h6 font-weight-bold">
                {{ priceTier }}
            </div>

            <div class="my-4 d-flex flex-column justify-center align-center">
                <div><span class="text-h4 font-weight-bold">{{ pricePerMonth }}</span></div>
                <div>USD per month</div>
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

            <sign-up-pricing-button
                class="my-4"
                :tier="tier"
                :annual="annual"
                :highlight="highlight"
                :currency="currency"
            >
            </sign-up-pricing-button>
        </v-sheet>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import SignUpPricingButton from '@client/vue/utility/squadov/SignUpPricingButton.vue'
import { Prop } from 'vue-property-decorator'
import { EPricingTier, FullPricingInfo, computePricePerMonth, Currency, formatCurrency } from '@client/js/squadov/pricing'

@Component({
    components: {
        SignUpPricingButton,
    }
})
export default class PricingTier extends Vue {
    @Prop({required: true})
    tier!: EPricingTier

    @Prop({type: Boolean, default: false})
    highlight!: boolean

    @Prop({required: true})
    pricing!: FullPricingInfo

    @Prop({type: Boolean, default: false})
    annual!: boolean

    @Prop({required: true})
    currency!: Currency

    get pricePerMonth(): string {
        let price = computePricePerMonth(this.pricing, this.tier)
        return formatCurrency(price, this.currency)
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
                    'Unlimited Squad Members',
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

.recommended-highlight {
    color: white;
    background-color: #4caf50;
    width: 80%;
    text-align: center;
    border-radius: 5px 5px 0 0;
}

.highlight-pricing {
    border: 2px #4caf50 solid;
}

</style>