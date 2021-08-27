<template>
    <v-container fluid>
        <div class="d-flex align-center">
            <h1>{{ title }}</h1>
            <v-btn icon @click="downloadData">
                <v-icon>
                    mdi-download
                </v-icon>
            </v-btn>
            <v-spacer></v-spacer>

            <div>
                <div class="d-flex align-center">
                    <span class="font-weight-bold mr-4">Cohort: </span>
                    <time-interval-select v-model="interval" class="mr-4"></time-interval-select>
                    <date-range-picker
                        :start-date.sync="startDate"
                        :end-date.sync="endDate"
                    ></date-range-picker>
                </div>

                <div class="d-flex align-center">
                    <span class="font-weight-bold mr-4">Period: </span>
                    <v-text-field
                        v-model.number="cohortLength"
                        type="number"
                        hide-details
                        single-line
                        class="mr-4"
                    >
                    </v-text-field>
                    <time-interval-select v-model="cohortPeriod"></time-interval-select>
                </div>
            </div>
        </div>
        <v-divider></v-divider>

        <template>
            <v-row>
                <v-col cols="12">
                    <v-data-table
                        v-if="!!data"
                        :headers="tableHeaders"
                        :items="tableData"
                        :items-per-page="30"
                    >
                    </v-data-table>
                    
                    <div class="d-flex justify-center" v-else>
                        <v-progress-circular size="64" indeterminate></v-progress-circular>
                    </div>
                </v-col>
            </v-row>
        </template>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { Interval, intervalToNounString, intervalToMilliseconds } from '@client/ts/interval'
import { Cohorts, CohortDatumContainer, getCohortData } from '@client/ts/cohorts'
import DateRangePicker from '@client/vue/DateRangePicker.vue'
import TimeIntervalSelect from '@client/vue/TimeIntervalSelect.vue'

@Component({
    components: {
        DateRangePicker,
        TimeIntervalSelect,
    }
})
export default class GenericMetricContainer extends Vue {
    @Prop({required: true})
    title!: string

    @Prop({required: true})
    cohort!: Cohorts

    $refs!: {
        graphDiv: HTMLElement
    }

    interval: Interval = Interval.Daily
    startDate: Date = new Date()
    endDate: Date = new Date()
    cohortPeriod: Interval = Interval.Daily
    cohortLength: number = 14

    get range(): Date[] {
        return [this.startDate, this.endDate]
    }

    data: CohortDatumContainer[] | null = null

    @Watch('metric')
    @Watch('interval')
    @Watch('startDate')
    @Watch('endDate')
    @Watch('cohortPeriod')
    @Watch('cohortLength')
    refreshData() {
        this.data = null

        getCohortData(this.cohort, this.interval, this.startDate, this.endDate, this.cohortPeriod, this.cohortLength).then((resp: CohortDatumContainer[]) => {
            this.data = resp
        }).catch((err: any) => {
            console.error('Failed to get cohort data: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }

    get tableHeaders(): any[] {
        let headers: any[] = [
            {
                text: 'Cohort',
                value: 'tm',
            },
            {
                text: 'Size',
                value: 'size',
            }
        ]

        for (let i = 0; i < this.cohortLength; ++i) {
            headers.push({
                text: `${intervalToNounString(this.cohortPeriod)} ${i}`,
                value: `c${i}`
            })
        }

        return headers
    }

    get tableData(): any[] {
        if (!this.data || this.data.length == 0) {
            return []
        }

        return this.data.map((ele: CohortDatumContainer) => {
            let ret: any = {
                tm: `${ele.tm.getUTCMonth()+1}/${ele.tm.getUTCDate()}/${ele.tm.getUTCFullYear()}`,
                size: ele.size,
            }

            for (let i = 0; i < this.cohortLength && i < ele.data.length; ++i) {
                let periodTm = new Date(ele.tm.getTime() + i * intervalToMilliseconds(this.cohortPeriod))
                if (periodTm > new Date()) {
                    ret[`c${i}`] = ''
                } else {
                    let percentage = ele.size > 0 ? (ele.data[i]/ele.size * 100) :  0
                    ret[`c${i}`] = `${(percentage).toFixed(1)}%`
                }
            }

            return ret
        })
    }

    downloadData() {
        let indices = new Array(this.cohortLength)
        for (let i = 0; i < this.cohortLength; ++i) {
            indices[i] = i
        }

        let content = 'data:text/csv;charset=utf-8,'
        content += this.tableHeaders.map((ele: any) => ele.text).join(',')
        content += '\r\n'
        content += this.tableData.map((ele: any) => [
            ele['tm'],
            ele['size'],
            ...indices.map((i : number) => ele[`c${i}`])
        ].join(',')).join('\r\n')

        let encoded = encodeURI(content)
        window.open(encoded)
    }
}

</script>