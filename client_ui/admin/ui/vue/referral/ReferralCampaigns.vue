<template>
    <div class="full-width">
        <div class="d-flex align-center ma-4">
            <v-text-field
                v-model="campaignCode"
                label="Code"
                solo
                dense
                hide-details
            >
            </v-text-field>

            <v-text-field
                class="ml-4"
                v-model="campaignDescription"
                label="Description"
                solo
                dense
                hide-details
            >
            </v-text-field>

            <v-btn
                class="ml-4"
                color="primary"
                :disabled="campaignCode.trim() == ''"
                @click="createCampaign"
                :loading="creating"
            >
                Create
            </v-btn>
        </div>

        <generic-referral-container
            title="Referral (Campaigns)"
            :metric="metric"
            :referral="referral"
        >
        </generic-referral-container>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import GenericReferralContainer from '@client/vue/referral/GenericReferralContainer.vue'
import { Metrics } from '@client/ts/metrics'
import { Referral, createCampaign } from '@client/ts/referrals'

@Component({
    components: {
        GenericReferralContainer
    }
})
export default class ReferralCampaigns extends Vue {
    campaignCode: string = ''
    campaignDescription: string = ''
    creating: boolean = false

    createCampaign() {
        this.creating = true
        createCampaign(this.campaignCode, this.campaignDescription).then(() => {
            this.campaignCode = ''
            this.campaignDescription = ''
            this.$router.go(0)
        }).catch((err: any) => {
            console.log('Failed to create campaign: ', err)
        }).finally(() => {
            this.creating = false
        })
    }

    get metric(): Metrics {
        return Metrics.ReferralCampaigns
    }

    get referral(): Referral {
        return Referral.Campaign
    }
}

</script>