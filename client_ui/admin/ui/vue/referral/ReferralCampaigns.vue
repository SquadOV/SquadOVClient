<template>
    <generic-referral-container
        title="Referral (Campaigns)"
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
export default class ReferralCampaigns extends Vue {
    get metric(): Metrics {
        return Metrics.ReferralCampaigns
    }

    get referral(): Referral {
        return Referral.Campaign
    }
}

</script>