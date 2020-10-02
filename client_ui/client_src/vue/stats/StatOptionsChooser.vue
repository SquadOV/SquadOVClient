<template>
    <div class="d-flex align-center justify-space-between">
        <template
            v-for="(opt, index) in availableOptions"
        >
            <v-select
                v-model="optionValues.values[opt.id]"
                dense
                :label="opt.name"
                v-if="!!opt.isEnum"
                :key="index"
                hide-details
                :items="getItemsForOption(opt)"
                class="mr-4"
                :value-comparator="cmpFn"
            >
            </v-select>
        </template>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { StatOption, StatValue, StatOptionValueMap } from '@client/js/stats/stat_library'

@Component
export default class StatOptionsChooser extends Vue {
    @Prop({required: true})
    optionValues!: StatOptionValueMap

    get availableOptions() : StatOption[] {
        return this.optionValues.options
    }

    getItemsForOption(opt : StatOption) : any[] {
        return opt.values.map((ele : StatValue) => ({
            text: ele.text,
            value: ele,
        }))
    }

    cmpFn(a : StatValue, b : any) : boolean {
        // I THINK THERE'S A BUG IN VUETIFY HERE.
        // b gets passed as StatValue.value rather than just StatValue...
        return a.value === b
    }
}

</script>

<style scoped>

</style>