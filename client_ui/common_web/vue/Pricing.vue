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

        <loading-container :is-loading="!finalPricingGrid">
            <template v-slot:default="{ loading }">
                <template v-if="!loading">
                    <v-row justify="center" align="center" class="mt-8">
                        <v-col cols="10">
                            <div class="d-flex justify-center align-center">
                                <div class="text-right" style="width: 170px;">
                                    <span>Monthly</span>
                                </div>
                                <v-switch
                                    class="mx-4"
                                    color="primary"
                                    v-model="annually"
                                >
                                </v-switch>
                                <div class="d-flex flex-column" style="width: 170px;">
                                    <div>
                                        Yearly
                                    </div>

                                    <div class="text-overline cyan--text">
                                        Save 15% Per Month!
                                    </div>
                                </div>
                            </div>

                            <div class="d-flex justify-center align-center mt-4">
                                <currency-selector
                                    v-model="selectedCurrency"
                                >
                                </currency-selector>
                            </div>

                            <v-row class="mt-8" justify="space-around" align="center">
                                <v-col cols="3">
                                    <pricing-tier
                                        :tier="EPricingTier.Basic"
                                        :pricing="finalPricingGrid"
                                        :currency="selectedCurrency"
                                        :annual="annually"
                                    >
                                    </pricing-tier>
                                </v-col>

                                <v-col cols="3">
                                    <pricing-tier
                                        :tier="EPricingTier.Silver"
                                        :pricing="finalPricingGrid"
                                        :currency="selectedCurrency"
                                        :annual="annually"
                                    >
                                    </pricing-tier>
                                </v-col>

                                <v-col cols="3">
                                    <pricing-tier
                                        :tier="EPricingTier.Gold"
                                        :pricing="finalPricingGrid"
                                        :currency="selectedCurrency"
                                        :annual="annually"
                                        highlight
                                    >
                                    </pricing-tier>
                                </v-col>

                                <v-col cols="3">
                                    <pricing-tier
                                        :tier="EPricingTier.Diamond"
                                        :pricing="finalPricingGrid"
                                        :currency="selectedCurrency"
                                        :annual="annually"
                                    >
                                    </pricing-tier>
                                </v-col>
                            </v-row>
                        </v-col>
                    </v-row>

                    <v-row justify="center" align="center">
                        <v-col cols="9" class="d-flex flex-column justify-center align-center">
                            <div class="text-h4 font-weight-bold text-center" style="width: 70%;">
                                Professionals and Teams
                            </div>

                            <div class="text-h6 mt-2 text-center" style="width: 60%;">
                                Content creator? Professional eSports team? Or just otherwise wanting to use SquadOV for non-individual purposes? Reach out and let us help!
                            </div>

                            <div class="d-flex justify-center align-center mt-4">
                                <div class="d-flex justify-center align-center mx-4">
                                    <v-icon class="mr-1" color="success">mdi-check-circle</v-icon>
                                    Custom Development
                                </div>

                                <div class="d-flex justify-center align-center mx-4">
                                    <v-icon class="mr-1" color="success">mdi-check-circle</v-icon>
                                    Private Deployment
                                </div>

                                <div class="d-flex justify-center align-center mx-4">
                                    <v-icon class="mr-1" color="success">mdi-check-circle</v-icon>
                                    Dedicated Support
                                </div>
                            </div>

                            <v-btn
                                class="mt-4"
                                @click="contactUs"
                                color="primary"
                                style="width: 20%;"
                            >
                                Contact Us
                            </v-btn>
                        </v-col>
                    </v-row>

                    <v-row justify="center" align="center">
                        <v-col cols="9">
                            <div class="d-flex justify-center align-center text-h6 font-weight-bold my-4">
                                <span>Full Feature Comparison</span>
                            </div>

                            <v-simple-table class="mb-8">
                                <template v-slot:default>
                                    <thead>
                                        <tr>
                                            <th class="text-left text-h6">
                                                Features
                                            </th>

                                            <th class="text-center text-h6">
                                                Basic
                                            </th>

                                            <th class="text-center text-h6">
                                                Silver
                                            </th>

                                            <th class="text-center text-h6">
                                                Gold
                                            </th>

                                            <th class="text-center text-h6">
                                                Diamond
                                            </th>
                                        </tr>
                                    </thead>

                                    <tbody>
                                        <tr>
                                            <td colspan="5" class="text-left font-weight-bold text-h5 my-4">
                                                Core Features
                                            </td>
                                        </tr>
                                        <tr>
                                            <td>Automatic Recording</td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                        </tr>
                                        <tr>
                                            <td>Advanced search and organization</td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                        </tr>
                                        <tr>
                                            <td>Easy and secure sharing</td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                        </tr>
                                        <tr>
                                            <td>POV Synchronization</td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                        </tr>
                                        <tr>
                                            <td>Support for Multiple Games</td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                        </tr>
                                        <tr>
                                            <td>Easy-to-Share Profiles</td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                        </tr>
                                        <tr>
                                            <td>No Ads</td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                            <td class="text-center"><v-icon color="success">mdi-check-circle</v-icon></td>
                                        </tr>

                                        <tr>
                                            <td colspan="5" class="text-left font-weight-bold text-h5 my-4">
                                                Video Quality
                                            </td>
                                        </tr>
                                        <tr>
                                            <td>Max Resolution</td>
                                            <td class="text-center">720p</td>
                                            <td class="text-center">1080p</td>
                                            <td class="text-center">1440p</td>
                                            <td class="text-center">Native</td>
                                        </tr>

                                        <tr>
                                            <td>Max FPS</td>
                                            <td class="text-center" colspan="2">
                                                60
                                            </td>
                                            <td class="text-center">90</td>
                                            <td class="text-center">144</td>
                                        </tr>

                                        <tr>
                                            <td>Max Bitrate (kbps)</td>
                                            <td class="text-center">6,000</td>
                                            <td class="text-center">12,000</td>
                                            <td class="text-center">18,000</td>
                                            <td class="text-center">24,000</td>
                                        </tr>

                                        <tr>
                                            <td>Video Codec</td>
                                            <td colspan="2" class="text-center">H.264</td>
                                            <td colspan="2" class="text-center">H.264, VP9</td>
                                        </tr>

                                        <tr>
                                            <td class="text-left font-weight-bold text-h5 my-4" colspan="5">
                                                Video Storage
                                            </td>
                                        </tr>
                                        <tr>
                                            <td>Retention (since recording)</td>
                                            <td class="text-center">7 days</td>
                                            <td class="text-center" colspan="3">
                                                Unlimited
                                            </td>
                                        </tr>

                                        <tr>
                                            <td>Cloud Storage Limit</td>
                                            <td class="text-center" colspan="4">
                                                Unlimited
                                            </td>
                                        </tr>

                                        <tr>
                                            <td>Downloadable</td>
                                            <td class="text-center" colspan="4">
                                                Unlimited
                                            </td>
                                        </tr>

                                        <tr>
                                            <td>Local Storage</td>
                                            <td class="text-center" colspan="4">
                                                Unlimited
                                            </td>
                                        </tr>

                                        <tr>
                                            <td colspan="5" class="text-left font-weight-bold text-h5 my-4">
                                                Audio Quality
                                            </td>
                                        </tr>

                                        <tr>
                                            <td>Bitrate (kbps)</td>
                                            <td colspan="4" class="text-center">128</td>
                                        </tr>

                                        <tr>
                                            <td>Audio Codec</td>
                                            <td colspan="2" class="text-center">AAC</td>
                                            <td colspan="2" class="text-center">AAC, Opus</td>
                                        </tr>

                                        <tr>
                                            <td colspan="5" class="text-left font-weight-bold text-h5 my-4">
                                                Clips
                                            </td>
                                        </tr>

                                        <tr>
                                            <td>Max Duration</td>
                                            <td class="text-center">
                                                2 minutes
                                            </td>
                                            <td class="text-center">
                                                3 minutes
                                            </td>
                                            <td class="text-center" colspan="2">
                                                5 minutes
                                            </td>
                                        </tr>

                                        <tr>
                                            <td>Retention</td>
                                            <td class="text-center" colspan="4">
                                                Unlimited
                                            </td>
                                        </tr>

                                        <tr>
                                            <td>Cloud Storage Limit</td>
                                            <td class="text-center" colspan="4">
                                                Unlimited
                                            </td>
                                        </tr>

                                        <tr>
                                            <td class="font-weight-bold text-left text-h5 my-4" colspan="5">
                                                SquadOV Extras
                                            </td>
                                        </tr>

                                        <tr>
                                            <td>Early Access</td>
                                            <td class="text-center" colspan="2">
                                                No
                                            </td>
                                            <td class="text-center">
                                                <v-icon color="success">mdi-check-circle</v-icon>
                                            </td>
                                            <td class="text-center">
                                                <v-icon color="success">mdi-check-circle</v-icon>
                                            </td>
                                        </tr>

                                        <tr>
                                            <td>VOD Processing Priority</td>
                                            <td class="text-center">
                                                Low
                                            </td>
                                            <td class="text-center" colspan="2">
                                                Medium
                                            </td>
                                            <td class="text-center">
                                                High
                                            </td>
                                        </tr>

                                        <tr>
                                            <td>Priority Support</td>
                                            <td class="text-center" colspan="2">
                                                No
                                            </td>
                                            <td class="text-center">
                                                <v-icon color="success">mdi-check-circle</v-icon>
                                            </td>
                                            <td class="text-center">
                                                <v-icon color="success">mdi-check-circle</v-icon>
                                            </td>
                                        </tr>

                                        <tr>
                                            <td>Video Watermarks</td>
                                            <td class="text-center">
                                                Yes
                                            </td>
                                            <td class="text-center" colspan="4">
                                                No
                                            </td>
                                        </tr>

                                        <tr>
                                            <td>Max Squad Size</td>
                                            <td class="text-center">
                                                20
                                            </td>
                                            <td class="text-center">
                                                100
                                            </td>
                                            <td class="text-center" colspan="2">
                                                Unlimited
                                            </td>
                                        </tr>

                                        <tr>
                                            <td class="font-weight-bold text-left text-h5 my-4" colspan="5">
                                                Discord Extras
                                            </td>
                                        </tr>
                                        <tr>
                                            <td>Custom Role</td>
                                            <td class="text-center">No</td>
                                            <td class="text-center">
                                                <v-icon color="success">mdi-check-circle</v-icon>
                                            </td>
                                            <td class="text-center">
                                                <v-icon color="success">mdi-check-circle</v-icon>
                                            </td>
                                            <td class="text-center">
                                                <v-icon color="success">mdi-check-circle</v-icon>
                                            </td>
                                        </tr>

                                        <tr>
                                            <td>Private Channels</td>
                                            <td class="text-center">No</td>
                                            <td class="text-center">
                                                <v-icon color="success">mdi-check-circle</v-icon>
                                            </td>
                                            <td class="text-center">
                                                <v-icon color="success">mdi-check-circle</v-icon>
                                            </td>
                                            <td class="text-center">
                                                <v-icon color="success">mdi-check-circle</v-icon>
                                            </td>
                                        </tr>

                                        <tr>
                                            <td>
                                                <div>
                                                    <v-radio-group
                                                        v-model="annually"
                                                        mandatory
                                                    >
                                                        <v-radio
                                                            label="Billed monthly"
                                                            :value="false"
                                                        >
                                                        </v-radio>

                                                        <v-radio
                                                            label="Billed yearly"
                                                            :value="true"
                                                        >
                                                        </v-radio>
                                                    </v-radio-group>
                                                </div>
                                            </td>

                                            <td class="text-center">
                                                <div class="my-4 d-flex flex-column justify-center align-center" v-if="!!finalPricingGrid">
                                                    <div><span class="text-h4 font-weight-bold">{{ formatCurrency(computePricePerMonth(finalPricingGrid, EPricingTier.Basic), selectedCurrency) }}</span></div>
                                                    <div>{{ selectedCurrency.toUpperCase() }} per month</div>
                                                    <sign-up-pricing-button
                                                        :tier="EPricingTier.Basic"
                                                        :annual="annually"
                                                        :currency="selectedCurrency"
                                                    >
                                                    </sign-up-pricing-button>
                                                </div>
                                            </td>

                                            <td class="text-center">
                                                <div class="my-4 d-flex flex-column justify-center align-center" v-if="!!finalPricingGrid">
                                                    <div><span class="text-h4 font-weight-bold">{{ formatCurrency(computePricePerMonth(finalPricingGrid, EPricingTier.Silver), selectedCurrency) }}</span></div>
                                                    <div>{{ selectedCurrency.toUpperCase() }} per month</div>
                                                    <sign-up-pricing-button
                                                        :tier="EPricingTier.Silver"
                                                        :annual="annually"
                                                        :currency="selectedCurrency"
                                                    >
                                                    </sign-up-pricing-button>
                                                </div>
                                            </td>

                                            <td class="text-center">
                                                <div class="my-4 d-flex flex-column justify-center align-center" v-if="!!finalPricingGrid">
                                                    <div><span class="text-h4 font-weight-bold">{{ formatCurrency(computePricePerMonth(finalPricingGrid, EPricingTier.Gold), selectedCurrency) }}</span></div>
                                                    <div>{{ selectedCurrency.toUpperCase() }} per month</div>
                                                    <sign-up-pricing-button
                                                        :tier="EPricingTier.Gold"
                                                        highlight
                                                        :annual="annually"
                                                        :currency="selectedCurrency"
                                                    >
                                                    </sign-up-pricing-button>
                                                </div>
                                            </td>

                                            <td class="text-center">
                                                <div class="my-4 d-flex flex-column justify-center align-center" v-if="!!finalPricingGrid">
                                                    <div><span class="text-h4 font-weight-bold">{{ formatCurrency(computePricePerMonth(finalPricingGrid, EPricingTier.Diamond), selectedCurrency) }}</span></div>
                                                    <div>{{ selectedCurrency.toUpperCase() }} per month</div>
                                                    <sign-up-pricing-button
                                                        class="mt-4"
                                                        :tier="EPricingTier.Diamond"
                                                        :annual="annually"
                                                        :currency="selectedCurrency"
                                                    >
                                                    </sign-up-pricing-button>
                                                </div>
                                            </td>
                                        </tr>
                                    </tbody>
                                </template>
                            </v-simple-table>
                        </v-col>
                    </v-row>
                </template>
            </template>
        </loading-container>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import PricingTier from '@client/vue/utility/squadov/PricingTier.vue'
import SignUpPricingButton from '@client/vue/utility/squadov/SignUpPricingButton.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import CurrencySelector from '@client/vue/utility/pricing/CurrencySelector.vue'
import { EPricingTier, FullPricingInfo, computePricePerMonth, Currency, formatCurrency } from '@client/js/squadov/pricing'
import { openUrlInBrowser } from '@client/js/external'
import { apiClient, ApiData } from '@client/js/api'

@Component({
    components: {
        PricingTier,
        LoadingContainer,
        SignUpPricingButton,
        CurrencySelector,
    }
})
export default class Pricing extends Vue {
    formatCurrency = formatCurrency
    EPricingTier = EPricingTier
    annually: boolean = false
    monthlyPricingGrid: FullPricingInfo | null = null
    yearlyPricingGrid: FullPricingInfo | null = null
    refreshInProgress: boolean = false
    selectedCurrency: Currency = Currency.Usd

    computePricePerMonth = computePricePerMonth

    get finalPricingGrid(): FullPricingInfo | null {
        if (this.annually) {
            return this.yearlyPricingGrid
        }
        return this.monthlyPricingGrid
    }

    @Watch('selectedCurrency')
    refreshPricing() {
        this.refreshInProgress = true
        apiClient.getPricingGrid(false, this.selectedCurrency).then((resp: ApiData<FullPricingInfo>) => {
            this.monthlyPricingGrid = resp.data
        }).finally(() => {
            this.refreshInProgress = false
        })

        apiClient.getPricingGrid(true, this.selectedCurrency).then((resp: ApiData<FullPricingInfo>) => {
            this.yearlyPricingGrid = resp.data
        })
    }

    mounted() {
        this.refreshPricing()
    }

    contactUs() {
        openUrlInBrowser('mailto:support@squadov.gg')
    }
}

</script>

<style scoped>

>>>td {
    border-left: thin solid rgba(255, 255, 255, 0.12);
    border-right: thin solid rgba(255, 255, 255, 0.12);
}

>>>th {
    border-left: thin solid rgba(255, 255, 255, 0.12);
    border-right: thin solid rgba(255, 255, 255, 0.12);
}

</style>