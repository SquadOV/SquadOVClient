<template>
    <v-container fluid>
        <div class="d-flex flex-wrap full-width justify-center">
            <div
                class="d-flex flex-column justify-center align-center kpi pa-2 ma-2"
                v-for="(kpi, idx) in allKpis"
                :key="idx"
            >
                <div class="text-h3 font-weight-bold">
                    <template v-if="!Number.isInteger(kpiMap[kpi])">
                        {{ kpiMap[kpi].toFixed(2) }}
                    </template>

                    <template v-else>
                        {{ formatBigNumber(kpiMap[kpi]) }}
                    </template>
                </div>

                <div class="d-flex flex-column justify-center align-center text-subtitle-2 font-weight-bold mt-2">
                    {{ kpiName(kpi) }}
                    <v-icon class="mt-2">
                        {{ kpiIcon(kpi) }}
                    </v-icon>
                </div>
            </div>
        </div>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Kpi, getKpi, kpiName, kpiIcon } from '@client/ts/kpi'
import { formatBigNumber } from '@client/ts/format'

@Component
export default class KpiDashboard extends Vue {
    Kpi: any = Kpi
    kpiName = kpiName
    kpiIcon = kpiIcon
    formatBigNumber = formatBigNumber

    kpiMap: { [kpi: number] : number } = {}

    get allKpis() : Kpi[] {
        return [
            Kpi.TotalUsers,
            Kpi.TotalVods,
            Kpi.TotalMatches,
            Kpi.ViralCoefficient,
        ]
    }

    refreshSingleKpi(kpi: Kpi) {
        getKpi(kpi).then((v: number) => {
            Vue.set(this.kpiMap, kpi, v)
        }).catch((err: any) => {
            console.log('Failed to get KPI: ', err)
        })
    }

    refreshKpis() {
        for (let kpi of this.allKpis) {
            this.refreshSingleKpi(kpi)
        }
    }

    mounted() {
        this.refreshKpis()
    }
}

</script>

<style scoped>

.kpi {
    width: 256px;
    height: 160px;
    border: 1px solid white;
    border-radius: 4px;
}

</style>