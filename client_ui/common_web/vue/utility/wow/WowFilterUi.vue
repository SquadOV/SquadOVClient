<template>
    <generic-match-filter-ui>
        <div class="d-flex align-center">
            <v-select
                v-if="forRaids"
                label="Encounters"
                v-model="internalValue.encounters"
                @input="syncToValue"
                :items="encounterItems"
                :loading="encounterItems === null"
                deletable-chips
                chips
                multiple
                clearable
                outlined
                hide-details
                dense
                style="max-width: 500px;"
            >
                <template v-slot:item="{ item }">
                    <div class="d-flex full-width align-center">
                        <v-checkbox
                            class="selection-checkbox"
                            dense
                            hide-details
                            :input-value="internalValue.encounters.includes(item.value)"
                            readonly
                        >
                        </v-checkbox>

                        {{ item.text }}
                    </div>
                </template>
            </v-select>

            <v-select
                v-if="forDungeons"
                label="Dungeons"
                v-model="internalValue.dungeons"
                @input="syncToValue"
                :items="dungeonItems"
                :loading="dungeonItems === null"
                deletable-chips
                chips
                multiple
                clearable
                outlined
                hide-details
                dense
                style="max-width: 500px;"
            >
                <template v-slot:item="{ item }">
                    <div class="d-flex full-width align-center">
                        <v-checkbox
                            class="selection-checkbox"
                            dense
                            hide-details
                            :input-value="internalValue.dungeons.includes(item.value)"
                            readonly
                        >
                        </v-checkbox>

                        {{ item.text }}
                    </div>
                </template>
            </v-select>

            <v-select
                v-if="forRaids"
                label="Raids"
                v-model="internalValue.raids"
                @input="syncToValue"
                :items="raidItems"
                :loading="raidItems === null"
                deletable-chips
                chips
                multiple
                clearable
                outlined
                hide-details
                dense
                style="max-width: 500px;"
            >
                <template v-slot:item="{ item }">
                    <div class="d-flex full-width align-center">
                        <v-checkbox
                            class="selection-checkbox"
                            dense
                            hide-details
                            :input-value="internalValue.raids.includes(item.value)"
                            readonly
                        >
                        </v-checkbox>

                        {{ item.text }}
                    </div>
                </template>
            </v-select>

            <v-select
                v-if="forArenas"
                label="Arenas"
                v-model="internalValue.arenas"
                @input="syncToValue"
                :items="arenaItems"
                :loading="arenaItems === null"
                deletable-chips
                chips
                multiple
                clearable
                outlined
                hide-details
                dense
                style="max-width: 500px;"
            >
                <template v-slot:item="{ item }">
                    <div class="d-flex full-width align-center">
                        <v-checkbox
                            class="selection-checkbox"
                            dense
                            hide-details
                            :input-value="internalValue.arenas.includes(item.value)"
                            readonly
                        >
                        </v-checkbox>

                        {{ item.text }}
                    </div>
                </template>
            </v-select>

            <v-checkbox
                class="ml-2 mt-0"
                label="Must have VOD"
                v-model="internalValue.hasVod"
                @change="syncToValue"
                hide-details
                dense
            ></v-checkbox>
        </div>
    </generic-match-filter-ui>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { WowMatchFilters, createEmptyWowMatchFilters } from '@client/js/wow/filters'
import { wowCache, WowContentDatum } from '@client/js/wow/staticCache'
import GenericMatchFilterUi from '@client/vue/utility/GenericMatchFilterUi.vue'

@Component({
    components: {
        GenericMatchFilterUi
    }
})
export default class WowFilterUi extends Vue {
    @Prop({required: true})
    value!: WowMatchFilters

    @Prop({type: Boolean, default: false})
    forRaids!: boolean

    @Prop({type: Boolean, default: false})
    forDungeons!: boolean

    @Prop({type: Boolean, default: false})
    forArenas!: boolean

    internalValue: WowMatchFilters = createEmptyWowMatchFilters()

    encounterItems: any[] | null = null
    dungeonItems: any[] | null = null
    raidItems: any[] | null = null
    arenaItems: any[] | null = null

    @Watch('value')
    syncFromValue() {
        this.internalValue = JSON.parse(JSON.stringify(this.value))
    }

    syncToValue() {
        this.$emit('input', JSON.parse(JSON.stringify(this.internalValue)))
    }

    mounted() {
        wowCache.getEncounters().then((resp: WowContentDatum[]) => {
            this.encounterItems = resp.map((ele: WowContentDatum) => {
                return {
                    text: ele.name,
                    value: ele.id,
                }
            }).sort((a, b) => {
                if (a.text < b.text) {
                    return -1
                } else if (a.text > b.text) {
                    return 1
                } else {
                    return 0
                }
            })
        })

        wowCache.getDungeons().then((resp: WowContentDatum[]) => {
            this.dungeonItems = resp.map((ele: WowContentDatum) => {
                return {
                    text: ele.name,
                    value: ele.id,
                }
            }).sort((a, b) => {
                if (a.text < b.text) {
                    return -1
                } else if (a.text > b.text) {
                    return 1
                } else {
                    return 0
                }
            })
        })

        wowCache.getRaids().then((resp: WowContentDatum[]) => {
            this.raidItems = resp.map((ele: WowContentDatum) => {
                return {
                    text: ele.name,
                    value: ele.id,
                }
            }).sort((a, b) => {
                if (a.text < b.text) {
                    return -1
                } else if (a.text > b.text) {
                    return 1
                } else {
                    return 0
                }
            })
        })

        wowCache.getArenas().then((resp: WowContentDatum[]) => {
            this.arenaItems = resp.map((ele: WowContentDatum) => {
                return {
                    text: ele.name,
                    value: ele.id,
                }
            }).sort((a, b) => {
                if (a.text < b.text) {
                    return -1
                } else if (a.text > b.text) {
                    return 1
                } else {
                    return 0
                }
            })
        })

        this.syncFromValue()
    }
}

</script>

<style scoped>

.selection-checkbox {
    margin: 0px !important;
}

</style>