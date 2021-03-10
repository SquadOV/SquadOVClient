<template>
    <v-container fluid>
        <div class="d-flex align-center">
            <h1>{{ title }}</h1>
            <v-spacer></v-spacer>
            <time-interval-select v-model="interval" class="mr-4"></time-interval-select>
            <date-range-picker
                :start-date.sync="startDate"
                :end-date.sync="endDate"
            ></date-range-picker>
        </div>
        <v-divider></v-divider>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { Interval } from '@client/ts/interval'
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

    interval: Interval = Interval.Daily
    startDate: Date = new Date()
    endDate: Date = new Date()
}

</script>