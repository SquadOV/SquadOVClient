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
                label="Must have VOD"
                v-model="internalValue.hasVod"
                @change="syncToValue"
                hide-details
                dense
            ></v-checkbox>

            <v-checkbox
                class="ml-2 mt-0"
                label="Must have Demo"
                v-model="internalValue.hasDemo"
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
import { CsgoMatchFilters, createEmptyCsgoMatchFilters } from '@client/js/csgo/filters'
import { csgo_maps } from '@client/js/csgo/data'
import GenericMatchFilterUi from '@client/vue/utility/GenericMatchFilterUi.vue'
import CommonFilters from '@client/vue/utility/CommonFilters'

@Component({
    components: {
        GenericMatchFilterUi
    }
})
export default class CsgoFilterUi extends mixins(CommonFilters) {
    @Prop({required: true})
    value!: CsgoMatchFilters

    internalValue: CsgoMatchFilters = createEmptyCsgoMatchFilters()

    get modeItems(): any[] {
        return [
            {
                value: 'casual',
                text: 'Casual',
            },
            {
                value: 'competitive',
                text: 'Competitive',
            },      
        ]
    }

    get mapItems(): any[] {
        return csgo_maps.map((ele: string) => {
            return {
                value: ele,
                text: ele,
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