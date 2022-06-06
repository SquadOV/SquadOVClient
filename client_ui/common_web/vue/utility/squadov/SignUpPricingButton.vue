<template>
    <v-btn
        :color="highlight ? 'success' : 'primary'"
        :to="redirectTo"
    >
        Sign Up
    </v-btn>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { EPricingTier } from '@client/js/squadov/pricing'
import { RegisterPageId, SubscriptionPageId } from '@client/js/pages'


@Component
export default class SignUpPricingButton extends Vue {
    @Prop({required: true})
    tier!: EPricingTier

    @Prop({type: Boolean, default: false})
    highlight!: boolean

    @Prop({type: Boolean, default: false})
    annual!: boolean

    get redirectTo() : any {
        let subTo = {
            name: SubscriptionPageId,
            query: {
                tier: this.tier,
                annual: this.annual ? '1' : '0',
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