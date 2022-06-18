<template>
    <div ref="parentDiv" class="d-flex flex-column justify-center align-center full-parent-height full-width">
        <template v-if="!success">
            <v-progress-circular size="64" indeterminate></v-progress-circular>
        </template>

        <template v-else>
            <div class="font-weight-bold text-h4 text-center">
                <span>Thank you for subscribing!</span>
            </div>

            <div class="text-h6 long-text text-center" style="width: 50%">
                <span>Your support means a lot - especially to a small team like ours. SquadOV is always trying to be better for users like yourself. Let us know what we can do to improve your VOD experience <a href="#" @click="goToFeedback">here</a>.</span>
            </div>

            <v-btn
                class="mt-4"
                color="primary"
                :to="to"                
            >
                Home
            </v-btn>
        </template>

        <v-snackbar
            v-model="failStart"
            :timeout="5000"
            color="error"
        >
            Failed to redirect you to the right location. Oops? Try again later (please).
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { Currency, EPricingTier } from '@client/js/squadov/pricing'
import { apiClient, ApiData } from '@client/js/api'
import { DashboardPageId, PricingPageId } from '@client/js/pages'
import confetti from 'canvas-confetti'
import { openUrlInBrowser } from '@client/js/external'

@Component
export default class SubscriptionWorkflow extends Vue {
    @Prop({default: EPricingTier.Gold})
    tier!: EPricingTier

    @Prop({type: Boolean, default: false})
    annual!: boolean

    @Prop({type: Boolean})
    success!: boolean | undefined

    @Prop({type: String})
    currency!: Currency

    failStart: boolean = false

    confettiTimeout: number | null = null

    $refs!: {
        canvas: HTMLCanvasElement,
    }

    goToFeedback() {
        openUrlInBrowser('https://feedback.squadov.gg/')
    }

    get to(): any {
        return {
            name: DashboardPageId,
        }
    }

    stopConfetti() {
        if (this.confettiTimeout !== null) {
            window.clearTimeout(this.confettiTimeout)
            this.confettiTimeout = null
        }
    }

    beforeDestroy() {
        this.stopConfetti()
    }

    deactivated() {
        this.stopConfetti()
    }

    shootConfetti() {
        confetti({
            resize: true,
            disableForReducedMotion: true,
            particleCount: Math.random() * 50 + 50,
            origin: {
                x: Math.random(),
                y: Math.random(),
            },
            startVelocity: 30,
            spread: 360,
            ticks: 60,
            zIndex: 0,
        })
    }

    startConfetti() {
        this.confettiTimeout = window.setTimeout(() => {
            this.shootConfetti()
            this.startConfetti()  
        }, 300 + Math.random() * 800)
    }

    mounted() {
        if (this.success === undefined) {
            // Make a call to our API server. Our API server will create a Stripe checkout session
            // that'll get sent back here. Once that happens, we'll redirect users back to the app (web or desktop).
            apiClient.startSubscription(this.tier, this.annual, this.currency).then((resp: ApiData<string>) => {
                window.location.href = resp.data
            }).catch((err: any) => {
                console.warn('Failed to start subscription workflow: ', err)
                this.failStart = true
            })
        } else if (!this.success) {
            // Just redirect users to the pricing page if user cancelled. Safe bet - probably.
            this.$router.replace({
                name: PricingPageId
            })
        } else {
            // If user subscribed, then we should thank for them for subscribing.
            this.startConfetti()   
        }
    }
}

</script>