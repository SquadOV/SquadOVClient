<template>
    <div class="d-flex justify-center align-center full-parent-height full-width">
        <v-progress-circular size="64" indeterminate></v-progress-circular>

        <v-snackbar
            v-model="failStart"
            :timeout="5000"
            color="error"
        >
            Failed to redirect you to the checkout page. Oops? Try again.
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { EPricingTier } from '@client/js/squadov/pricing'
import { apiClient, ApiData } from '@client/js/api'

@Component
export default class SubscriptionWorkflow extends Vue {
    @Prop({default: EPricingTier.Gold})
    tier!: EPricingTier

    @Prop({type: Boolean, default: false})
    annual!: boolean

    @Prop({type: Boolean})
    success!: boolean | undefined

    failStart: boolean = false

    mounted() {
        // Make a call to our API server. Our API server will create a Stripe checkout session
        // that'll get sent back here. Once that happens, we'll redirect users back to the app (web or desktop).
        if (this.success === undefined) {
            apiClient.startSubscription(this.tier, this.annual).then((resp: ApiData<string>) => {
                window.location.href = resp.data
            }).catch((err: any) => {
                console.warn('Failed to start subscription workflow: ', err)
                this.failStart = true
            })
        }
    }
}

</script>