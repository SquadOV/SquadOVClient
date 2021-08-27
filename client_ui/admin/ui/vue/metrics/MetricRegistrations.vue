<template>
    <generic-metric-container
        title="Registrations"
        :metric="metric"
        :extra="extra"
    >
        <v-autocomplete
            label="Referral Codes"
            v-model="selectedReferralCodes"
            :items="availableReferralCodes"
            deletable-chips
            chips
            multiple
            clearable
            outlined
            hide-details
            style="max-width: 500px;"
        >
        </v-autocomplete>
    </generic-metric-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import GenericMetricContainer from '@client/vue/metrics/GenericMetricContainer.vue'
import { Metrics } from '@client/ts/metrics'
import { getAvailableReferralCodes } from '@client/ts/referrals'

@Component({
    components: {
        GenericMetricContainer,
    }
})
export default class MetricRegistrations extends Vue {
    availableReferralCodes: string[] = []

    refreshReferralCodes() {
        getAvailableReferralCodes().then((resp: string[]) => {
            this.availableReferralCodes = resp.sort((a: string, b: string) => {
                if (a < b) {
                    return -1
                } else if (a > b) {
                    return 1
                } else {
                    return 0
                }
            })
        }).catch((err: any) => {
            console.error('Failed to get referral codes: ', err)
        })
    }

    selectedReferralCodes: string[] = []

    get extra(): any {
        return {
            'referralCodes': JSON.stringify(this.selectedReferralCodes),
        }
    }

    get metric(): Metrics {
        return Metrics.Registrations
    }

    mounted() {
        this.refreshReferralCodes()
    }
}

</script>