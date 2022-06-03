<template>
    <v-container fluid>
        <v-row justify="center" align="center">
            <v-col cols="6">
                <div class="d-flex flex-column justify-center align-center">
                    <div class="text-h2 font-weight-bold mt-4">
                        SquadOV Pro
                    </div>

                    <div class="text-h4 mt-2">
                        Because <span class="font-weight-bold">YOU</span> deserve better VODs.
                    </div>

                    <div class="text-h6 mt-4">
                        Find the right moments when you need them faster than ever before.
                    </div>
                </div>
            </v-col>
        </v-row>

        <v-row justify="center" align="center" class="mt-8">
            <v-col cols="10">
                <v-row justify="space-around" align="center">
                    <v-col cols="3">
                        <pricing-tier
                            :tier="EPricingTier.Basic"
                        >
                        </pricing-tier>
                    </v-col>

                    <v-col cols="3">
                        <pricing-tier
                            :tier="EPricingTier.Silver"
                        >
                        </pricing-tier>
                    </v-col>

                    <v-col cols="3">
                        <pricing-tier
                            :tier="EPricingTier.Gold"
                            highlight
                        >
                        </pricing-tier>
                    </v-col>

                    <v-col cols="3">
                        <pricing-tier
                            :tier="EPricingTier.Diamond"
                        >
                        </pricing-tier>
                    </v-col>
                </v-row>
            </v-col>
        </v-row>

        <v-row justify="center" align="center">
            <v-col cols="9">
                <v-sheet
                    rounded
                >
                    Professionals and Teams
                </v-sheet>
            </v-col>
        </v-row>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import PricingTier from '@client/vue/utility/squadov/PricingTier.vue'
import { EPricingTier } from '@client/js/squadov/pricing'

@Component({
    components: {
        PricingTier
    }
})
export default class Pricing extends Vue {
    EPricingTier = EPricingTier
}

</script>