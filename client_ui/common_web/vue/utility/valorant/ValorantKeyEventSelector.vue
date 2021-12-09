<template>
    <v-select
        :value="value"
        @input="$emit('input', arguments[0])"
        :items="items"
        clearable
        hide-details
        outlined
    >
    </v-select>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { ALL_VALORANT_MATCH_FILTER_EVENTS, ValorantMatchFilterEvents, filterEventToName } from '@client/js/valorant/filters'

@Component
export default class ValorantKeyEventSelector extends Vue {
    @Prop({required: true})
    value!: ValorantMatchFilterEvents[]

    get items(): any[] {
        return ALL_VALORANT_MATCH_FILTER_EVENTS.map((ele) => {
            return {
                text: filterEventToName(ele),
                value: ele,
            }
        })
    }
}

</script>