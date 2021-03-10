<template>
    <generic-referral-container
        title="Referral (Users)"
        :metric="metric"
        :referral="referral"
    >
    </generic-referral-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import GenericReferralContainer from '@client/vue/referral/GenericReferralContainer.vue'
import { Metrics } from '@client/ts/metrics'
import { Referral } from '@client/ts/referrals'

@Component({
    components: {
        GenericReferralContainer
    }
})
export default class ReferralUsers extends Vue {
    get metric(): Metrics {
        return Metrics.ReferralUsers
    }

    get referral(): Referral {
        return Referral.User
    }
}

</script>