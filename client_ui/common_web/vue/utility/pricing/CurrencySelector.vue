<template>
    <v-select
        :value="value"
        class="flex-grow-0"
        dense
        :items="options"
        @input="$emit('input', arguments[0])"
        hide-details
        outlined
        style="max-width: 100px;"
    >
        <template v-slot:item="{ item }">
            <div class="d-flex full-width align-center">
                {{ item.text }}&nbsp;<img :src="`https://flagcdn.com/16x12/${item.icon}.png`"></img>
            </div>
        </template>

        <template v-slot:selection="{ item }">
            <div class="d-flex full-width align-center">
                {{ item.text }}&nbsp;<img :src="`https://flagcdn.com/16x12/${item.icon}.png`"></img>
            </div>
        </template>
    </v-select>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { Currency } from '@client/js/squadov/pricing'

@Component
export default class CurrencySelector extends Vue {
    @Prop({required: true})
    value!: Currency

    get options(): any[] {
        return [
            {
                text: 'USD',
                value: Currency.Usd,
                icon: 'us',
            },
            {
                text: 'EUR',
                value: Currency.Euro,
                icon: 'eu',
            }
        ]
    }
}

</script>