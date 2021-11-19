<template>
    <div class="d-flex align-center">
        <generic-match-filter-ui>
            <div class="d-flex align-center">
                <v-select
                    label="Tasks"
                    v-model="internalValue.tasks"
                    @input="syncToValue"
                    :items="taskItems"
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
                                :input-value="
                                    internalValue.tasks.includes(item.value)
                                "
                                readonly
                            >
                            </v-checkbox>

                            {{ item.text }}

                            <v-spacer></v-spacer>

                            <v-img
                                :src="$root.generateAssetUri(item.icon)"
                                :width="32"
                                :max-width="32"
                                :height="32"
                                :max-height="32"
                                contain
                            >
                            </v-img>
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
        <v-spacer></v-spacer>
        <v-btn :to="vizStatsPageTo">
            Stat Visualization
        </v-btn>
    </div>
</template>

<script lang="ts">

import Component, {mixins} from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { AimlabMatchFilters, createEmptyAimlabMatchFilters } from '@client/js/aimlab/filters'
import { AimlabContent, getAimlabContent } from '@client/js/aimlab/aimlab_content'
import GenericMatchFilterUi from '@client/vue/utility/GenericMatchFilterUi.vue'
import CommonFilters from '@client/vue/utility/CommonFilters'
import * as pi from '@client/js/pages'

@Component({
    components: {
        GenericMatchFilterUi
    }
})
export default class AimlabFilterUi extends mixins(CommonFilters) {
    @Prop({ required: true })
    value!: AimlabMatchFilters

    internalValue: AimlabMatchFilters = createEmptyAimlabMatchFilters()

    get content(): AimlabContent {
        return getAimlabContent()
    }

    get vizStatsPageTo(): any {
        return {
            name: pi.VizStatsPageId
        }
    }

    get taskItems(): any[] {
        return this.content.validTaskKeys.map((k: string) => {
            return {
                value: k,
                text: this.content.getTaskName(k),
                icon: this.content.getTaskIcon(k),
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