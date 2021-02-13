<template>
    <generic-riot-user-chooser @on-account-change="$emit('on-account-change')" :puuid="puuid" :account="account">
        <v-select
            :value="value"
            @input="$emit('input', arguments[0])"
            label="Summoner"
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
import { RiotSummoner } from '@client/js/riot/summoner'
import GenericRiotUserChooser from '@client/vue/utility/riot/GenericRiotUserChooser.vue'

@Component({
    components: {
        GenericRiotUserChooser
    }
})
export default class SummonerAccountChooser extends Vue {
    @Prop({required: true})
    value! : RiotSummoner | null

    @Prop({type: Array, required: true})
    options! : RiotSummoner[]

    get items() : any[] {
        return this.options!.map((ele : RiotSummoner) => ({
            text: ele.summonerName,
            value: ele,
        }))
    }

    get puuid(): string | null {
        if (!this.value) {
            return null
        }
        return this.value.puuid
    }

    get account(): string | null {
        if (!this.value) {
            return null
        }
        return this.value.summonerName
    }
}

</script>

<style scoped>

</style>