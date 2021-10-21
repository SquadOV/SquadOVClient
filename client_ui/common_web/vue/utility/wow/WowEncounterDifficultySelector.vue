<template>
    <v-select
        label="Difficulty"
        :value="value"
        @input="$emit('input', arguments[0])"
        :items="items"
        deletable-chips
        chips
        multiple
        clearable
        outlined
        hide-details
        dense
        style="max-width: 500px;"
    >
    </v-select>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { ALL_WOW_ENCOUNTER_DIFFICULTIES, encounterDifficultyToName } from '@client/js/wow/difficulty'

@Component
export default class WowEncounterDifficultySelector extends Vue {
    @Prop()
    value!: number[]

    get items(): any[] {
        return ALL_WOW_ENCOUNTER_DIFFICULTIES.map((ele: number) => {
            return {
                text: encounterDifficultyToName(ele),
                value: ele,
            }
        })
    }
}

</script>