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

            <v-checkbox
                v-model="organicOnly"
                dense
                hide-details
                label="Organic Only"
            >
            </v-checkbox>

            <v-checkbox
                v-model="verifiedOnly"
                dense
                hide-details
                label="Verified Only"
            >
            </v-checkbox>
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
            console.error('Failed to get referral codes: ', err)
        })
    }

    selectedReferralCodes: string[] = []
    startDate: Date = new Date()
    endDate: Date = new Date()
    data: FunnelData | null = null
    organicOnly: boolean = false
    verifiedOnly: boolean = false

    @Watch('selectedReferralCodes')
    @Watch('startDate')
    @Watch('endDate')
    @Watch('organicOnly')
    @Watch('verifiedOnly')
    refreshData() {
        getFunnelData(this.startDate, this.endDate, this.selectedReferralCodes, this.organicOnly, this.verifiedOnly).then((resp: FunnelData) => {
            this.data = resp
        }).catch((err: any) => {
            console.error('Failed to get funnel data: ', err)
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
                phase: 'Login Web',
                count: this.data.loginweb,
                pprev: `${(this.data.loginweb / this.data.reg * 100.0).toFixed(2)}%`,
                pinit: `${(this.data.loginweb / this.data.view * 100.0).toFixed(2)}%`,
            },
            {
                phase: 'Login Desktop',
                count: this.data.logindesktop,
                pprev: `${(this.data.logindesktop / this.data.loginweb * 100.0).toFixed(2)}%`,
                pinit: `${(this.data.logindesktop / this.data.view * 100.0).toFixed(2)}%`,
            },
            {
                phase: 'HW Sync',
                count: this.data.hwsync,
                pprev: `${(this.data.hwsync / this.data.logindesktop * 100.0).toFixed(2)}%`,
                pinit: `${(this.data.hwsync / this.data.view * 100.0).toFixed(2)}%`,
            },
            {
                phase: 'Ready Record',
                count: this.data.readyrecord,
                pprev: `${(this.data.readyrecord / this.data.hwsync * 100.0).toFixed(2)}%`,
                pinit: `${(this.data.readyrecord / this.data.view * 100.0).toFixed(2)}%`,
            },
            {
                phase: 'Launch Game',
                count: this.data.launchgame,
                pprev: `${(this.data.launchgame / this.data.readyrecord * 100.0).toFixed(2)}%`,
                pinit: `${(this.data.launchgame / this.data.view * 100.0).toFixed(2)}%`,
            },
            {
                phase: 'Start Record',
                count: this.data.startrecord,
                pprev: `${(this.data.startrecord / this.data.launchgame * 100.0).toFixed(2)}%`,
                pinit: `${(this.data.startrecord / this.data.view * 100.0).toFixed(2)}%`,
            },
            {
                phase: 'Record',
                count: this.data.record,
                pprev: `${(this.data.record / this.data.startrecord * 100.0).toFixed(2)}%`,
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