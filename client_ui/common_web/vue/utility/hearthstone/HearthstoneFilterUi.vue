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

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { HearthstoneMatchFilters, createEmptyHearthstoneMatchFilters } from '@client/js/hearthstone/filters'
import GenericMatchFilterUi from '@client/vue/utility/GenericMatchFilterUi.vue'

@Component({
    components: {
        GenericMatchFilterUi
    }
})
export default class HearthstoneFilterUi extends Vue {
    @Prop({required: true})
    value!: HearthstoneMatchFilters

    internalValue: HearthstoneMatchFilters = createEmptyHearthstoneMatchFilters()

    @Watch('value')
    syncFromValue() {
        this.internalValue = JSON.parse(JSON.stringify(this.value))
    }

    syncToValue() {
        this.$emit('input', JSON.parse(JSON.stringify(this.internalValue)))
    }

    mounted() {
        this.syncFromValue()
    }
}

</script>