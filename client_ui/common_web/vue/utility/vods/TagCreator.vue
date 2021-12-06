<template>
    <v-combobox
        :value="value"
        @input="$emit('input', arguments[0])"
        dense
        single-line
        hide-details
        multiple
        chips
        small-chips
        deletable-chips
        clearable
        outlined
    >
    </v-combobox>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'

@Component
export default class TagCreator extends Vue {
    @Prop({required: true})
    value!: string[]
}

</script>