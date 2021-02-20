<template>
    <div class="d-flex align-center justify-space-between">
        <template
            v-for="(opt, index) in option.options"
        >
            <v-select
                v-model="option.data[opt.id]"
                dense
                :label="opt.name"
                :key="index"
                hide-details
                :items="getItemsForOption(opt)"
                class="mr-4"
            >
            </v-select>
        </template>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { StatOptionInstance, StatOption, StatOptionChoice } from '@client/js/stats/statPrimitives'

@Component
export default class StatOptionChooser extends Vue {
    @Prop()
    option!: StatOptionInstance

    getItemsForOption(opt: StatOption) : any[] {
        return opt.choices!.map((ele : StatOptionChoice) => ({
           text: ele.name,
           value: ele.value, 
        }))
    }
}

</script>