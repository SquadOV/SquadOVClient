<template>
<v-container fluid>
        <div class="d-flex align-center">
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
                :items="data"
                :items-per-page="30"
                style="width: 60%;"
            >
                <template v-slot:item.perc="{ item }">
                    {{ (item.perc * 100.0).toFixed(2) }}%
                </template>
            </v-data-table>
        </div>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import DateRangePicker from '@client/vue/DateRangePicker.vue'
import { Watch } from 'vue-property-decorator'
import { getRegistrationReferralBreakdown, RegistrationReferralCount } from '@client/ts/referrals'

@Component({
    components: {
        DateRangePicker,
    }
})
export default class RegistrationBreakdown extends Vue {
    startDate: Date = new Date()
    endDate: Date = new Date()
    data: RegistrationReferralCount[] = []

    get tableHeaders(): any[] {
        return [
            {
                text: 'Referral Code',
                value: 'code'
            },
            {
                text: 'Count',
                value: 'count'
            },
            {
                text: 'Percentage',
                value: 'perc'
            },
        ]
    }

    @Watch('startDate')
    @Watch('endDate')
    refreshData() {
        this.data = []
        getRegistrationReferralBreakdown(this.startDate, this.endDate).then((resp: RegistrationReferralCount[]) => {
            this.data = resp
        }).catch((err: any) => {
            console.error('Failed to get reg referral breakdown: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>