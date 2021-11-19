<template>
    <generic-match-filter-ui>
        <div class="d-flex align-center">
            <v-select
                label="Maps"
                v-model="tmpMaps"
                :items="mapItems"
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
                            :input-value="tmpMaps.includes(item.value)"
                            readonly
                        >
                        </v-checkbox>

                        {{ item.text }}
                    </div>
                </template>
            </v-select>

            <v-select
                label="Modes"
                v-model="internalValue.modes"
                @input="syncToValue"
                :items="modeItems"
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
                            :input-value="internalValue.modes.includes(item.value)"
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

import Component, {mixins} from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { LolMatchFilters, createEmptyLolMatchFilters } from '@client/js/lol/filters'
import GenericMatchFilterUi from '@client/vue/utility/GenericMatchFilterUi.vue'
import maps from '@client/js/lol/maps.json'
import gameModes from '@client/js/lol/gameModes.json'
import CommonFilters from '@client/vue/utility/CommonFilters'

@Component({
    components: {
        GenericMatchFilterUi
    }
})
export default class LolFilterUi extends mixins(CommonFilters) {
    @Prop({required: true})
    value!: LolMatchFilters

    internalValue: LolMatchFilters = createEmptyLolMatchFilters()
    tmpMaps: string[] = []

    @Watch('tmpMaps')
    syncTmpMaps() {
        this.internalValue.maps = this.tmpMaps.map((map: string) => {
            if (this.mapToIds.has(map)) {
                return this.mapToIds.get(map)!
            } else {
                return []
            }
        }).flat()
        this.syncToValue()
    }

    get uniqueMaps(): string[] {
        return Array.from(new Set(maps.map((ele: any) => ele.mapName))).sort((a, b) => {
            if (a < b) {
                return -1
            } else if (a > b) {
                return 1
            } else {
                return 0
            }
        })
    }

    get mapToIds(): Map<string, number[]> {
        let ret: Map<string, number[]> = new Map()
        for (let m of maps) {
            if (!ret.has(m.mapName)) {
                ret.set(m.mapName, [])
            }

            let arr = ret.get(m.mapName)!
            arr.push(m.mapId)
        }
        return ret
    }

    get mapItems(): any[] {
        return this.uniqueMaps.map((ele: string) => {
            return {
                value: ele,
                text: ele,
            }
        })
    }

    get modeItems(): any[] {
        return gameModes.map((ele: any) => {
            return {
                value: ele.gameMode,
                text: ele.description,
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
    }

    @Watch('value')
    syncFromValue() {
        this.internalValue = JSON.parse(JSON.stringify(this.value))
    }

    syncToValue() {
        this.$emit('input', JSON.parse(JSON.stringify(this.internalValue)))
        this.saveToLocal(this.internalValue)
    }

    mounted() {
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