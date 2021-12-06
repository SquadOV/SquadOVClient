<template>
    <div class="full-width">
        <generic-match-filter-ui>
            <div class="d-flex align-center">
                <div class="font-weight-bold mr-2">
                    Tags:
                </div>

                <tag-creator
                    v-model="internalValue.tags"
                    @input="syncToValue"
                >
                </tag-creator>
            </div>

            <div class="d-flex align-center mt-1">
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

        <generic-match-filter-ui
            class="mt-2"
            label="Per-Game Filters"
        >
            <v-tabs>
                <v-tab>
                    <v-img
                        width="32px"
                        max-width="32px"
                        :src="$root.generateAssetUri('assets/wow-logo.png')"
                        contain
                    >
                    </v-img>
                </v-tab>

                <v-tab-item>
                    <div class="ma-4">
                        <div>
                            <div class="font-weight-bold">
                                Encounters
                            </div>
                            <wow-filter-ui
                                v-model="internalValue.filters.wow.encounters"
                                @input="syncToValue"
                                :for-raids="true"
                                :release="WowGameRelease.Retail"
                                hide-vod-option
                                show-enabled
                            >
                            </wow-filter-ui>
                        </div>

                        <div class="mt-2">
                            <div class="font-weight-bold">
                                Keystones
                            </div>
                            <wow-filter-ui
                                v-model="internalValue.filters.wow.keystones"
                                @input="syncToValue"
                                :for-dungeons="true"
                                :release="WowGameRelease.Retail"
                                hide-vod-option
                                show-enabled
                            >
                            </wow-filter-ui>
                        </div>

                        <div class="mt-2">
                            <div class="font-weight-bold">
                                Arena
                            </div>
                            <wow-filter-ui
                                v-model="internalValue.filters.wow.arenas"
                                @input="syncToValue"
                                :for-arenas="true"
                                :release="WowGameRelease.Retail"
                                hide-vod-option
                                show-enabled
                            >
                            </wow-filter-ui>
                        </div>
                    </div>
                </v-tab-item>
            </v-tabs>
        </generic-match-filter-ui>
    </div>
</template>

<script lang="ts">

import Component, {mixins} from 'vue-class-component'
import CommonFilters from '@client/vue/utility/CommonFilters'
import { Watch, Prop } from 'vue-property-decorator'
import { RecentMatchFilters, createEmptyRecentMatchFilters } from '@client/js/squadov/recentMatch'
import GameFilterUi from '@client/vue/utility/squadov/filters/GameFilterUi.vue'
import WowReleaseFilterUi from '@client/vue/utility/squadov/filters/WowReleaseFilterUi.vue'
import SquadFilterUi from '@client/vue/utility/squadov/filters/SquadFilterUi.vue'
import UserFilterUi from '@client/vue/utility/squadov/filters/UserFilterUi.vue'
import TimeRangeFilterUi from '@client/vue/utility/squadov/filters/TimeRangeFilterUi.vue'
import GenericMatchFilterUi from '@client/vue/utility/GenericMatchFilterUi.vue'
import WowFilterUi from '@client/vue/utility/wow/WowFilterUi.vue'
import TagCreator from '@client/vue/utility/vods/TagCreator.vue'
import { SquadOvGames } from '@client/js/squadov/game'
import { WowGameRelease } from '@client/js/staticData'

@Component({
    components: {
        GenericMatchFilterUi,
        GameFilterUi,
        SquadFilterUi,
        UserFilterUi,
        TimeRangeFilterUi,
        WowReleaseFilterUi,
        WowFilterUi,
        TagCreator,
    }
})
export default class RecentMatchFiltersUi extends mixins(CommonFilters) {
    WowGameRelease: any = WowGameRelease

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