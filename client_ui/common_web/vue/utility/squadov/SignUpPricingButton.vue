<template>
    <v-btn
        :color="highlight ? 'success' : 'primary'"
        :to="redirectTo"
    >
        {{ text }}
    </v-btn>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { Currency, EPricingTier } from '@client/js/squadov/pricing'
import { RegisterPageId, SubscriptionPageId } from '@client/js/pages'
import { canUserTrial } from '@client/js/squadov/user'


@Component
export default class SignUpPricingButton extends Vue {
    @Prop({required: true})
    tier!: EPricingTier

    @Prop({type: Boolean, default: false})
    highlight!: boolean

    @Prop({type: Boolean, default: false})
    annual!: boolean

    @Prop({required: true})
    currency!: Currency

    get text(): string {
        if (this.tier == EPricingTier.Basic) {
            return 'Register'
        } else if (!!this.$store.state.currentUser && canUserTrial(this.$store.state.currentUser)) {
            return 'Free Trial'
        } else {
            return 'Sign Up'
        }
    }

    get redirectTo() : any {
        let subTo = {
            name: SubscriptionPageId,
            query: {
                tier: this.tier,
                annual: this.annual ? '1' : '0',
                currency: this.currency,
            }
        }

        if (!!this.$store.state.currentUser) {
            return subTo
        }

        return {
            name: RegisterPageId,
            query: {
                redirect: this.$router.resolve(subTo).resolved.fullPath,
            }
        }
    }
}

</script>