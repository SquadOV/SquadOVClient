<template>
    <generic-match-filter-ui>
        <div class="d-flex align-center">
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

            <v-select
                label="Maps"
                v-model="internalValue.maps"
                @input="syncToValue"
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
                            :input-value="internalValue.maps.includes(item.value)"
                            readonly
                        >
                        </v-checkbox>

                        {{ item.text }}
                    </div>
                </template>
            </v-select>

            <v-checkbox
                class="ml-2 mt-0"
                label="Must be Ranked"
                v-model="internalValue.isRanked"
                @change="syncToValue"
                hide-details
                dense
            ></v-checkbox>

            <v-checkbox
                class="ml-2 mt-0"
                label="Must have VOD"
                v-model="internalValue.hasVod"
                v-if="!hideVodOption"
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
import { ValorantMatchFilters, createEmptyValorantMatchFilters, migrateValorantMatchFilters } from '@client/js/valorant/filters'
import { getValorantContent, ValorantContent } from '@client/js/valorant/valorant_content'
import GenericMatchFilterUi from '@client/vue/utility/GenericMatchFilterUi.vue'
import CommonFilters from '@client/vue/utility/CommonFilters'

@Component({
    components: {
        GenericMatchFilterUi
    }
})
export default class ValorantFilterUi extends mixins(CommonFilters) {
    @Prop({required: true})
    value!: ValorantMatchFilters

    @Prop({type: Boolean, default: false})
    hideVodOption!: boolean

    internalValue: ValorantMatchFilters = createEmptyValorantMatchFilters()

    get content(): ValorantContent {
        return getValorantContent(null)
    }

    get modeItems(): any[] {
        return this.content.availableModes.map((md: string) => {
            return {
                value: md,
                text: this.content.gameModeToName(md),
            }
        })
    }

    get mapItems(): any[] {
        return this.content.availableMaps.map((md: string) => {
            return {
                value: md,
                text: this.content.mapAssetPathToName(md),
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

        migrateValorantMatchFilters(this.internalValue)
        this.syncToValue()
    }
}

</script>

<style scoped>

.selection-checkbox {
    margin: 0px !important;
}

</style>