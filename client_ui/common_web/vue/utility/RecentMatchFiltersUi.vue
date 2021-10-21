<template>
    <generic-match-filter-ui>
        <div class="d-flex align-center">
            <game-filter-ui
                v-model="internalValue.games"
                @input="syncToValue"
            >
            </game-filter-ui>
            <wow-release-filter-ui
                v-if="needsWowReleaseFilter"
                v-model="internalValue.wowReleases"
                @input="syncToValue"
            >
            </wow-release-filter-ui>
            <squad-filter-ui
                v-if="!disableSquads"
                v-model="internalValue.squads"
                @input="syncToValue"
                class="ml-1"
            >
            </squad-filter-ui>
            <user-filter-ui
                v-if="!disableUsers"
                v-model="internalValue.users"
                :squads="internalValue.squads"
                @input="syncToValue"
                class="ml-1"
            >
            </user-filter-ui>
            
        </div>

        <div class="d-flex align-center mt-1">
            <time-range-filter-ui
                :start.sync="internalValue.timeStart"
                :end.sync="internalValue.timeEnd"
                @on-change="syncToValue"
            ></time-range-filter-ui>
        </div>
    </generic-match-filter-ui>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { RecentMatchFilters, createEmptyRecentMatchFilters } from '@client/js/squadov/recentMatch'
import GameFilterUi from '@client/vue/utility/squadov/filters/GameFilterUi.vue'
import WowReleaseFilterUi from '@client/vue/utility/squadov/filters/WowReleaseFilterUi.vue'
import SquadFilterUi from '@client/vue/utility/squadov/filters/SquadFilterUi.vue'
import UserFilterUi from '@client/vue/utility/squadov/filters/UserFilterUi.vue'
import TimeRangeFilterUi from '@client/vue/utility/squadov/filters/TimeRangeFilterUi.vue'
import GenericMatchFilterUi from '@client/vue/utility/GenericMatchFilterUi.vue'
import { SquadOvGames } from '@client/js/squadov/game'
@Component({
    components: {
        GenericMatchFilterUi,
        GameFilterUi,
        SquadFilterUi,
        UserFilterUi,
        TimeRangeFilterUi,
        WowReleaseFilterUi,
    }
})
export default class RecentMatchFiltersUi extends Vue {
    @Prop({required: true})
    value!: RecentMatchFilters

    @Prop({type: Boolean, default: false})
    disableSquads!: boolean

    @Prop({type: Boolean, default: false})
    disableUsers!: boolean

    internalValue: RecentMatchFilters = createEmptyRecentMatchFilters()

    get needsWowReleaseFilter(): boolean {
        return !!this.internalValue.games && this.internalValue.games.includes(SquadOvGames.WorldOfWarcraft)
    }

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