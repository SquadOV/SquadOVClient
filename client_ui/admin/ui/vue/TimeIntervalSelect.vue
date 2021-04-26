<template>
    <v-select
        class="flex-grow-0"
        dense
        :items="items"
        :value="value"
        @input="$emit('input', arguments[0])"
        hide-details
    >
    </v-select>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { Interval } from '@client/ts/interval'

@Component
export default class TimeIntervalSelect extends Vue {
    @Prop({required: true})
    value!: Interval

    get items(): any[] {
        return [
            {
                'text': 'Daily',
                'value': Interval.Daily,
            },
            {
                'text': 'Weekly',
                'value': Interval.Weekly,
            },
            {
                'text': 'Monthly',
                'value': Interval.Monthly,
            }
        ]
    }
}

</script>