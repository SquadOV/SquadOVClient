<template>
    <generic-match-filter-ui>
        <div class="d-flex align-center">
            <v-checkbox
                class="mt-0 mr-2"
                label="Enabled"
                v-if="showEnabled"
                v-model="internalValue.enabled"
                @change="syncToValue"
                hide-details
                dense
            >
            </v-checkbox>

            <v-select
                v-if="forInstances"
                label="Instance Type"
                v-model="internalValue.instanceTypes"
                @input="syncToValue"
                :items="instanceTypeItems"
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
                            :input-value="internalValue.instanceTypes.includes(item.value)"
                            readonly
                        >
                        </v-checkbox>

                        {{ item.text }}
                    </div>
                </template>
            </v-select>

            <v-select
                v-if="forRaids || forInstances"
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
                v-if="forRaids"
                label="Encounters"
                v-model="internalValue.encounters"
                @input="syncToValue"
                :items="filteredEncounteredItems"
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
                v-if="forDungeons || forInstances"
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
                v-if="forArenas"
                label="Brackets"
                v-model="internalValue.brackets"
                @input="syncToValue"
                :items="bracketItems"
                :loading="bracketItems === null"
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
                            :input-value="internalValue.brackets.includes(item.value)"
                            readonly
                        >
                        </v-checkbox>

                        {{ item.text }}
                    </div>
                </template>
            </v-select>

            <v-select
                v-if="forArenas || forInstances"
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

            <v-select
                class="ml-2"
                label="Success"
                v-model="internalValue.isWinner"
                :items="successItems"
                @change="syncToValue"
                hide-details
                outlined
                dense
                v-if="!forInstances"
            >
            </v-select>

            <v-checkbox
                class="ml-2 mt-0"
                label="Must have VOD"
                v-if="!hideVodOption"
                v-model="internalValue.hasVod"
                @change="syncToValue"
                hide-details
                dense
            >
            </v-checkbox>
        </div>

        <div class="d-flex align-center mt-2" v-if="isRetail">
            <number-range-filter-ui
                v-if="forArenas"
                label="Rating"
                :lower.sync="internalValue.ratingLow"
                @update:lower="syncToValue"
                :upper.sync="internalValue.ratingHigh"
                @update:upper="syncToValue"
            >
            </number-range-filter-ui>

            <number-range-filter-ui
                v-if="forDungeons"
                label="Keystones"
                :lower.sync="internalValue.keystoneLow"
                @update:lower="syncToValue"
                :upper.sync="internalValue.keystoneHigh"
                @update:upper="syncToValue"
            >
            </number-range-filter-ui>

            <wow-encounter-difficulty-selector
                v-if="forRaids"
                v-model="internalValue.encounterDifficulties"
                @input="syncToValue"
            >
            </wow-encounter-difficulty-selector>

            <div class="d-flex align-center ml-4" v-if="!forInstances">
                <div>
                    POV:
                </div>

                <wow-spec-chooser
                    class="ml-1"
                    v-model="internalValue.povSpec"
                    @input="syncToValue"
                >
                </wow-spec-chooser>
            </div>

            <wow-composition-selector
                class="ml-4"
                label="Team 1"
                v-model="internalValue.friendlyComposition"
                v-if="!forInstances"
                @input="syncToValue"
            >
            </wow-composition-selector>

            <wow-composition-selector
                class="ml-4"
                label="Team 2"
                v-model="internalValue.enemyComposition"
                v-if="forArenas"
                @input="syncToValue"
            >
            </wow-composition-selector>
        </div>
    </generic-match-filter-ui>
</template>

<script lang="ts">

import Component, {mixins} from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { WowMatchFilters, createEmptyWowMatchFilters } from '@client/js/wow/filters'
import { wowInstanceTypeToName, ALL_WOW_INSTANCE_TYPES, WowInstanceType } from '@client/js/wow/matches'
import { wowCache, WowContentDatum } from '@client/js/wow/staticCache'
import GenericMatchFilterUi from '@client/vue/utility/GenericMatchFilterUi.vue'
import NumberRangeFilterUi from '@client/vue/utility/squadov/filters/NumberRangeFilterUi.vue'
import WowSpecChooser from '@client/vue/utility/wow/WowSpecChooser.vue'
import WowEncounterDifficultySelector from '@client/vue/utility/wow/WowEncounterDifficultySelector.vue'
import WowCompositionSelector from '@client/vue/utility/wow/WowCompositionSelector.vue'
import { WowGameRelease } from '@client/js/staticData'
import CommonFilters from '@client/vue/utility/CommonFilters'

@Component({
    components: {
        GenericMatchFilterUi,
        NumberRangeFilterUi,
        WowSpecChooser,
        WowEncounterDifficultySelector,
        WowCompositionSelector,
    }
})
export default class WowFilterUi extends mixins(CommonFilters) {
    @Prop({required: true})
    value!: WowMatchFilters

    @Prop({type: Boolean, default: false})
    forRaids!: boolean

    @Prop({type: Boolean, default: false})
    forDungeons!: boolean

    @Prop({type: Boolean, default: false})
    forArenas!: boolean

    @Prop({type: Boolean, default: false})
    forInstances!: boolean

    @Prop({required: true})
    release!: WowGameRelease

    // Recent matches always have VODs which is when we turn this on.
    @Prop({type: Boolean, default: false})
    hideVodOption!: boolean

    @Prop({type: Boolean, default: false})
    showEnabled!: boolean

    internalValue: WowMatchFilters = createEmptyWowMatchFilters()

    encounterItems: any[] | null = null
    dungeonItems: any[] | null = null
    raidItems: any[] | null = null
    arenaItems: any[] | null = null

    get isRetail(): boolean {
        return this.release === WowGameRelease.Retail
    }

    @Watch('value')
    syncFromValue() {
        this.internalValue = JSON.parse(JSON.stringify(this.value))
    }

    syncToValue() {
        this.$emit('input', JSON.parse(JSON.stringify(this.internalValue)))
        this.saveToLocal(this.internalValue)
    }

    get successItems(): any[] {
        return [
            {
                text: this.forArenas ? 'Win' : 'Clear',
                value: true,
            },
            {
                text: this.forArenas ? 'Loss' : 'Wipe',
                value: false,
            },
            {
                text: 'All',
                value: null,
            }
        ]
    }

    get bracketItems(): any[] {
        return [
            {
                text: '2v2',
                value: '2v2',
            },
            {
                text: '3v3',
                value: '3v3',
            },
            {
                text: 'Skirmish',
                value: 'Skirmish',
            },
            {
                text: 'War Game',
                value: '5v5',
            },
        ]
    }

    encounterToRaid: Map<number, number> = new Map()

    get filteredEncounteredItems(): any[] {
        if (!this.encounterItems) {
            return []
        }

        if (this.internalValue.raids.length > 0) {
            let raidSet = new Set(this.internalValue.raids)
            return this.encounterItems.filter((ele: any) => {
                let raidId = this.encounterToRaid.get(ele.value)
                if (raidId === undefined) {
                    return false
                }
                return raidSet.has(raidId)
            })
        } else {
            return this.encounterItems
        }
    }

    get instanceTypeItems(): any[] {
        return ALL_WOW_INSTANCE_TYPES.map((ele: WowInstanceType) => {
            return {
                text: wowInstanceTypeToName(ele),
                value: ele,
            }
        })
    }

    mounted() {
        wowCache.getEncounters(this.release).then((resp: WowContentDatum[]) => {
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

            resp.forEach((ele: WowContentDatum) => {
                if (ele.parent === null) {
                    return
                }

                this.encounterToRaid.set(ele.id, ele.parent)
            })
        })

        wowCache.getDungeons(this.release).then((resp: WowContentDatum[]) => {
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

        wowCache.getRaids(this.release).then((resp: WowContentDatum[]) => {
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

        wowCache.getArenas(this.release).then((resp: WowContentDatum[]) => {
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
        if (!!this.savedFilter) {
            this.internalValue = JSON.parse(JSON.stringify(this.savedFilter))
            this.syncToValue()
        }
    }
}

</script>

<style scoped>

.selection-checkbox {
    margin: 0px !important;
}

</style>