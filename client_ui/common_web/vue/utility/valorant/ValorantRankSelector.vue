<template>
    <v-select
        :value="value"
        @input="$emit('input', arguments[0])"
        :items="rankItems"
        clearable
        hide-details
        outlined
        style="max-width: 250px;"
    >
        <template v-slot:item="{item}">
            <valorant-rank-tier
                :value="item.value"
                show-text
            >
            </valorant-rank-tier>
        </template>

        <template v-slot:selection="{item}">
            <valorant-rank-tier
                :value="item.value"
                show-text
            >
            </valorant-rank-tier>
        </template>
    </v-select>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import ValorantRankTier from '@client/vue/utility/valorant/ValorantRankTier.vue'
import { VALID_VALORANT_RANKS } from '@client/js/valorant/valorant_utility'

@Component({
    components: {
        ValorantRankTier
    }
})
export default class ValorantRankSelector extends Vue {
    @Prop({required: true})
    value!: number | undefined

    get rankItems(): any[] {
        return VALID_VALORANT_RANKS.map((ele) => ({
            value: ele.tier
        }))
    }
}

</script>