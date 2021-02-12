<template>
    <generic-riot-user-chooser @on-account-change="$emit('on-account-change')" :puuid="puuid" :account="account">
        <v-select
            :value="value"
            @input="$emit('input', arguments[0])"
            label="Valorant Account"
            :items="items"
            solo
            hide-details
        >
        </v-select>
    </generic-riot-user-chooser>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { RiotAccountData } from '@client/js/valorant/valorant_account'
import GenericRiotUserChooser from '@client/vue/utility/riot/GenericRiotUserChooser.vue'

@Component({
    components: {
        GenericRiotUserChooser,
    }
})
export default class RiotAccountChooser extends Vue {
    @Prop({required: true})
    value! : RiotAccountData | null

    @Prop({type: Array, required: true})
    options! : RiotAccountData[]

    get puuid(): string | null {
        if (!this.value) {
            return null
        }
        return this.value.puuid
    }

    get items() : any[] {
        return this.options!.map((ele : RiotAccountData) => ({
            text: `${ele.gameName}#${ele.tagLine}`,
            value: ele,
        }))
    }

    get account(): string | null {
        if (!this.value) {
            return null
        }
        return `${this.value.gameName}#${this.value.tagLine}`
    }
}

</script>

<style scoped>

</style>