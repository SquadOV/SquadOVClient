<template>
    <v-container fluid>
        <div class="d-flex align-center">
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
            <date-range-picker
                class="ml-4"
                :start-date.sync="startDate"
                :end-date.sync="endDate"
            ></date-range-picker>
        </div>

        <div class="d-flex flex-wrap full-width justify-center mt-4">
            <v-data-table
                v-if="!!data"
                :headers="tableHeaders"
                :items="tableData"
                :items-per-page="30"
                style="width: 50%;"
            >
            </v-data-table>
        </div>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import DateRangePicker from '@client/vue/DateRangePicker.vue'
import { getAvailableReferralCodes } from '@client/ts/referrals'
import { Watch } from 'vue-property-decorator'
import { FunnelData, getFunnelData } from '@client/ts/funnel'

@Component({
    components: {
        DateRangePicker,
    }
})
export default class Funnel extends Vue {
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
            console.log('Failed to get referral codes: ', err)
        })
    }

    selectedReferralCodes: string[] = []
    startDate: Date = new Date()
    endDate: Date = new Date()
    data: FunnelData | null = null

    @Watch('selectedReferralCodes')
    @Watch('startDate')
    @Watch('endDate')
    refreshData() {
        getFunnelData(this.startDate, this.endDate, this.selectedReferralCodes).then((resp: FunnelData) => {
            this.data = resp
        }).catch((err: any) => {
            console.log('Failed to get funnel data: ', err)
        })
    }

    get tableHeaders(): any[] {
        return [
            {
                text: 'Phase',
                value: 'phase'
            },
            {
                text: 'Count',
                value: 'count'
            },
            {
                text: '% of Previous',
                value: 'pprev'
            },
            {
                text: '% of Initial',
                value: 'pinit'
            },
        ]
    }

    get tableData(): any[] {
        if (!this.data) {
            return []
        }
        return [
            {
                phase: 'View',
                count: this.data.view,
                pprev: '100%',
                pinit: '100%',
            },
            {
                phase: 'Registration',
                count: this.data.reg,
                pprev: `${(this.data.reg / this.data.view * 100.0).toFixed(2)}%`,
                pinit: `${(this.data.reg / this.data.view * 100.0).toFixed(2)}%`,
            },
            {
                phase: 'Login',
                count: this.data.login,
                pprev: `${(this.data.login / this.data.reg * 100.0).toFixed(2)}%`,
                pinit: `${(this.data.login / this.data.view * 100.0).toFixed(2)}%`,
            },
            {
                phase: 'Download',
                count: this.data.download,
                pprev: `${(this.data.download / this.data.login * 100.0).toFixed(2)}%`,
                pinit: `${(this.data.download / this.data.view * 100.0).toFixed(2)}%`,
            },
            {
                phase: 'Install',
                count: this.data.install,
                pprev: `${(this.data.install / this.data.download * 100.0).toFixed(2)}%`,
                pinit: `${(this.data.install / this.data.view * 100.0).toFixed(2)}%`,
            },
            {
                phase: 'Record',
                count: this.data.record,
                pprev: `${(this.data.record / this.data.install * 100.0).toFixed(2)}%`,
                pinit: `${(this.data.record / this.data.view * 100.0).toFixed(2)}%`,
            },
            {
                phase: 'Active',
                count: this.data.active,
                pprev: `${(this.data.active / this.data.record * 100.0).toFixed(2)}%`,
                pinit: `${(this.data.active / this.data.view * 100.0).toFixed(2)}%`,
            },
        ]
    }

    mounted() {
        this.refreshReferralCodes()
        this.refreshData()
    }
}

</script>