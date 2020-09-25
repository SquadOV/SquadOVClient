<template>
    <v-select
        :value="value"
        @input="$emit('input', arguments[0])"
        label="Valorant Account"
        :items="items"
        solo
        hide-details
    >
    </v-select>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { ValorantAccountData } from '@client/js/valorant/valorant_account'

@Component
export default class ValorantAccountChooser extends Vue {
    @Prop({required: true})
    value! : ValorantAccountData

    @Prop({type: Array, required: true})
    options! : ValorantAccountData[]

    get items() : any[] {
        return this.options!.map((ele : ValorantAccountData) => ({
            text: `${ele.username}#${ele.tag}`,
            value: ele,
        }))
    }
}

</script>

<style scoped>

</style>