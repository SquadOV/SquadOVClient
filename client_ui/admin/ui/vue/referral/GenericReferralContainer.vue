<template>
    <v-container fluid>
        <v-tabs vertical>
            <v-tab>
                Breakdown
            </v-tab>

            <v-tab-item>
                <v-data-table
                    v-if="!!data"
                    :headers="referralHeaders"
                    :items="referralItems"
                    :items-per-page="30"
                >
                </v-data-table>

                <div class="d-flex justify-center" v-else>
                    <v-progress-circular size="64" indeterminate></v-progress-circular>
                </div>
            </v-tab-item>

            <v-tab>
                Timeline
            </v-tab>

            <v-tab-item>
                <generic-metric-container
                    :title="title"
                    :metric="metric"
                >
                </generic-metric-container>
            </v-tab-item>
        </v-tabs>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { Referral, ReferralDatum, getReferralBreakdown } from '@client/ts/referrals'
import { Metrics } from '@client/ts/metrics'
import GenericMetricContainer from '@client/vue/metrics/GenericMetricContainer.vue'

@Component({
    components: {
        GenericMetricContainer
    }
})
export default class GenericReferralContainer extends Vue {
    @Prop({required: true})
    title!: string

    @Prop({required: true})
    referral!: Referral

    @Prop({required: true})
    metric!: Metrics

    data: ReferralDatum[] | null = null
    @Watch('referral')
    refreshData() {
        this.data = null
        getReferralBreakdown(this.referral).then((resp: ReferralDatum[]) => {
            this.data = resp
        }).catch((err: any) => {
            console.error('Failed to get referral data: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }

    get referralHeaders(): any[] {
        return [
            {
                text: 'Code',
                value: 'code',
            },
            {
                text: 'Description',
                value: 'description',
            },
            {
                text: 'Visits',
                value: 'visits',
            },
            {
                text: 'Downloads',
                value: 'downloads',
            }
        ]
    }

    get referralItems(): any[] {
        if (!this.data) {
            return []
        }
        return this.data
    }
}

</script>