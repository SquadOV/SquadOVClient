<template>
    <div>
        <template v-if="showFilters">
            <v-btn
                class="pa-0"
                text
                @click="showFilters = false"
                x-small
            >
                <v-icon x-small>
                    mdi-chevron-down
                </v-icon>
                Hide Filters
            </v-btn>
        </template>

        <template v-else>
            <v-btn
                class="pa-0"
                text
                @click="showFilters = true"
                x-small
            >
                <v-icon x-small>
                    mdi-chevron-right
                </v-icon>
                Show Filters
            </v-btn>
        </template>

        <div v-if="showFilters">
            <div class="d-flex align-center">
                <game-filter-ui
                    v-model="internalValue.games"
                    @input="syncToValue"
                    class="mr-1"
                ></game-filter-ui>
                <squad-filter-ui
                    v-model="internalValue.squads"
                    @input="syncToValue"
                    class="mr-1"
                ></squad-filter-ui>
                <user-filter-ui
                    v-model="internalValue.users"
                    :squads="internalValue.squads"
                    @input="syncToValue"
                    class="mr-1"
                ></user-filter-ui>
                
            </div>

            <div class="d-flex align-center mt-1">
                <time-range-filter-ui
                    :start.sync="internalValue.timeStart"
                    :end.sync="internalValue.timeEnd"
                    @on-change="syncToValue"
                ></time-range-filter-ui>
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { RecentMatchFilters, createEmptyRecentMatchFilters } from '@client/js/squadov/recentMatch'
import GameFilterUi from '@client/vue/utility/squadov/filters/GameFilterUi.vue'
import SquadFilterUi from '@client/vue/utility/squadov/filters/SquadFilterUi.vue'
import UserFilterUi from '@client/vue/utility/squadov/filters/UserFilterUi.vue'
import TimeRangeFilterUi from '@client/vue/utility/squadov/filters/TimeRangeFilterUi.vue'

@Component({
    components: {
        GameFilterUi,
        SquadFilterUi,
        UserFilterUi,
        TimeRangeFilterUi,
    }
})
export default class RecentMatchFiltersUi extends Vue {
    @Prop({required: true})
    value!: RecentMatchFilters

    internalValue: RecentMatchFilters = createEmptyRecentMatchFilters()
    showFilters: boolean = false

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