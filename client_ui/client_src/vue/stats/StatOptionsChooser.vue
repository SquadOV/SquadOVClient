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
                :items="opt.values"
                class="mr-4"
            >
            </v-select>
        </template>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { StatOption, StatOptionValueMap } from '@client/js/stats/stat_library'

@Component
export default class StatOptionsChooser extends Vue {
    @Prop({required: true})
    optionValues!: StatOptionValueMap

    get availableOptions() : StatOption[] {
        return this.optionValues.options
    }
}

</script>

<style scoped>

</style>