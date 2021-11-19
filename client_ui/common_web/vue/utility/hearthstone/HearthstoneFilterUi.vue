<template>
    <generic-match-filter-ui>
        <div class="d-flex align-center">
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
import { HearthstoneMatchFilters, createEmptyHearthstoneMatchFilters } from '@client/js/hearthstone/filters'
import GenericMatchFilterUi from '@client/vue/utility/GenericMatchFilterUi.vue'
import CommonFilters from '@client/vue/utility/CommonFilters'

@Component({
    components: {
        GenericMatchFilterUi
    }
})
export default class HearthstoneFilterUi extends mixins(CommonFilters) {
    @Prop({required: true})
    value!: HearthstoneMatchFilters

    internalValue: HearthstoneMatchFilters = createEmptyHearthstoneMatchFilters()

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