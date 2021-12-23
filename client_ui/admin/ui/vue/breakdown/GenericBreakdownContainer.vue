<template>
    <v-container fluid>
        <div class="d-flex align-center">
            <h1>{{ title }}</h1>
            <slot></slot>
            <v-spacer></v-spacer>
            <date-range-picker
                :start-date.sync="startDate"
                :end-date.sync="endDate"
            ></date-range-picker>
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
import { Breakdown, BreakdownDatumContainer, getBreakdownData } from '@client/ts/breakdown'
import DateRangePicker from '@client/vue/DateRangePicker.vue'

@Component({
    components: {
        DateRangePicker,
    }
})
export default class GenericBreakdownContainer extends Vue {
    @Prop({required: true})
    title!: string

    @Prop({required: true})
    breakdown!: Breakdown

    @Prop()
    extra!: any

    $refs!: {
        graphDiv: HTMLElement
    }

    startDate: Date = new Date()
    endDate: Date = new Date()
    data: BreakdownDatumContainer| null = null

    @Watch('metric')
    @Watch('interval')
    @Watch('startDate')
    @Watch('endDate')
    @Watch('extra')
    refreshData() {
        this.data = null

        getBreakdownData(this.breakdown, this.startDate, this.endDate, this.extra).then((resp: BreakdownDatumContainer) => {
            this.data = resp
        }).catch((err: any) => {
            console.error('Failed to get breakdown data: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }

    get tableHeaders(): any[] {
        if (!this.data) {
            return []
        }

        return this.data.headers.map((ele: string) => {
            return {
                text: ele,
                value: ele,
            }
        })
    }

    get tableData(): any[] {
        if (!this.data) {
            return []
        }

        let ret = this.data.data.map((ele: any[]) => {
            let obj: any = {}
            this.data!.headers.forEach((h: string, idx: number) => {
                obj[h] = ele[idx]
            })
            return obj
        })
        return ret
    }
}

</script>