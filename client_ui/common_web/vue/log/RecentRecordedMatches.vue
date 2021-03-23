<template>
    <div>
        <div class="d-flex text-h6 align-center">
            {{ title }}

            <v-btn icon @click="refreshData">
                <v-icon>
                    mdi-refresh
                </v-icon>
            </v-btn>

            <v-spacer></v-spacer>

            <template v-if="!inSelectMode">
                <v-btn
                    color="primary"
                    @click="inSelectMode = true"
                >
                    Select
                </v-btn>
            </template>

            <template v-else>
                <v-dialog persistent v-model="showHideDelete" max-width="60%">
                    <template v-slot:activator="{on, attrs}">
                        <v-btn
                            icon
                            color="error"
                            :disabled="selected.length === 0"
                            v-on="on"
                            v-bind="attrs"
                        >
                            <v-icon>
                                mdi-delete
                            </v-icon>
                        </v-btn>
                    </template>

                    <v-card>
                        <v-card-title>
                            Are you sure?
                        </v-card-title>

                        <v-card-text>
                            Are you sure you want to delete {{ selected.length }} VODs?
                            This action can not be undone.
                        </v-card-text>

                        <v-card-actions>
                            <v-btn
                                color="error"
                                @click="showHideDelete = false"
                            >
                                Cancel
                            </v-btn>

                            <v-spacer></v-spacer>

                            <v-btn
                                color="success"
                                @click="deleteSelectedVods"
                                :loading="deleteInProgress"
                            >
                                Delete
                            </v-btn>
                        </v-card-actions>
                    </v-card>
                </v-dialog>

                <v-btn
                    @click="toggleSelected"
                    outlined
                    class="ml-2"
                >
                    <v-checkbox
                        :input-value="allSelected"
                        readonly
                    >
                    </v-checkbox>

                    {{ selected.length }} Selected
                </v-btn>

                <v-btn
                    class="ml-2"
                    icon
                    @click="closeSelect"
                    color="warning"
                >
                    <v-icon>
                        mdi-close
                    </v-icon>
                </v-btn>
            </template>
        </div>
        <v-divider class="my-2"></v-divider>
        <recent-match-filters-ui
            v-model="filters"
            :disable-squads="isUserLocked"
            :disable-users="isUserLocked"
        ></recent-match-filters-ui>
        
        <v-list-item-group v-model="selected" multiple>
            <loading-container :is-loading="!recentMatches">
                <template v-slot:default="{ loading }">
                    <template v-if="!loading && recentMatches.length > 0">
                        <template v-for="(group, gidx) in groupedMatches">
                            <div class="d-flex align-center my-2" :key="`group-header-${gidx}`">
                                <v-divider class="mr-4"></v-divider>

                                <div class="text-caption">
                                    Played {{ group.days }}
                                </div>
                            </div>

                            <div class="full-width" :key="`group-matches-${gidx}`">
                                <v-list-item
                                    class="pa-0"
                                    v-for="(match, idx) in group.matches"
                                    :key="idx"
                                    :value="match.base.vod.videoTracks[0].metadata.videoUuid"
                                >
                                    <template v-slot="{active}">
                                        <div class="d-flex align-center full-width">
                                            <v-list-item-action v-if="inSelectMode">
                                                <v-checkbox :input-value="active"></v-checkbox>
                                            </v-list-item-action>

                                            <recent-match-display
                                                class="mb-4 full-width"
                                                :match="match"
                                                :disable-click="inSelectMode"
                                            >
                                            </recent-match-display>
                                        </div>
                                    </template>
                                    
                                </v-list-item>
                            </div>
                        </template>

                        <v-btn
                            v-if="hasNext"
                            color="primary"
                            block
                            @click="loadMoreMatches"
                            :loading="loading"
                        >
                            Load More
                        </v-btn>
                    </template>

                    <div class="d-flex justify-center align-center full-parent-height full-width long-text" v-else>
                        <div class="text-h6">No recently recorded matches found for you or your squadmates! Start playing some games to get SquadOV to automatically record VODs. Don't forget to invite your friends!</div>
                    </div>
                </template>
            </loading-container>
        </v-list-item-group>

        <v-snackbar
            v-model="showHideDeleteError"
            :timeout="5000"
            color="error"
        >
            Failed to delete VODs. Please try again.
        </v-snackbar>
    </div>
</template>

<script lang="ts">

const maxTasksPerRequest : number = 10

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { RecentMatch, checkRecentMatchValidity, RecentMatchFilters, createEmptyRecentMatchFilters } from '@client/js/squadov/recentMatch'
import { apiClient, HalResponse, ApiData } from '@client/js/api'
import { numDaysAgo, numDaysAgoToString } from '@client/js/time'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import RecentMatchDisplay from '@client/vue/utility/RecentMatchDisplay.vue'
import RecentMatchFiltersUi from '@client/vue/utility/RecentMatchFiltersUi.vue'

interface GroupedMatch {
    days: string
    matches: RecentMatch[]
}

@Component({
    components: {
        LoadingContainer,
        RecentMatchDisplay,
        RecentMatchFiltersUi,
    }
})
export default class RecentRecordedMatches extends Vue {
    @Prop({default: 'Recent VODs'})
    title!: string

    @Prop()
    userId!: number | undefined

    @Prop({type: Boolean, default: false})
    onlyFavorite!: boolean

    @Prop({type: Boolean, default: false})
    onlyWatchlist!: boolean

    recentMatches: RecentMatch[] | null = null
    lastIndex: number = 0
    nextLink: string | null = null
    filters: RecentMatchFilters = createEmptyRecentMatchFilters()
    loading: boolean = false

    inSelectMode: boolean = false
    selected: string[] = []
    deleteInProgress: boolean = false
    showHideDelete: boolean = false
    showHideDeleteError: boolean = false

    get isUserLocked(): boolean {
        return this.userId !== undefined
    }

    get hasNext() : boolean {
        return !!this.nextLink
    }

    get selectableMatches(): RecentMatch[] {
        return this.filteredMatches.filter((ele: RecentMatch) => {
            return ele.base.userId === this.$store.state.currentUser.id
        })
    }

    get displayMatches(): RecentMatch[] {
        if (this.inSelectMode) {
            return this.selectableMatches
        } else {
            return this.filteredMatches
        }
    }

    get filteredMatches() : RecentMatch[] {
        if (!this.recentMatches) {
            return []
        }

        return this.recentMatches.filter((ele: RecentMatch) => {
            return checkRecentMatchValidity(ele)
        })
    }

    get groupedMatches(): GroupedMatch[] {
        let grouped: Map<number, RecentMatch[]> = new Map()
        this.displayMatches.forEach((ele: RecentMatch) => {
            let days = numDaysAgo(ele.base.tm)
            if (!grouped.has(days)) {
                grouped.set(days, [])
            }

            let arr = grouped.get(days)!
            arr.push(ele)
            grouped.set(days, arr)
        })

        let ret: GroupedMatch[] = []
        for (const [days, matches] of grouped) {
            ret.push({
                days: numDaysAgoToString(days),
                matches
            })
        }
        return ret
    }

    get allSelected(): boolean {
        return this.recentMatches?.length === this.selected.length
    }

    toggleSelected() {
        if (this.allSelected) {
            this.selected = []
        } else if (!!this.recentMatches) {
            this.selected = this.recentMatches.map((ele: RecentMatch) => ele.base.vod.videoTracks[0].metadata.videoUuid)
        } else {
            this.selected = []
        }
    }

    closeSelect() {
        this.inSelectMode = false
        this.selected = []
    }

    get finalFilters(): RecentMatchFilters {
        let filters: RecentMatchFilters = JSON.parse(JSON.stringify(this.filters))

        if (this.userId !== undefined) {
            filters.users = [this.userId]
        }

        filters.onlyFavorite = this.onlyFavorite
        filters.onlyWatchlist = this.onlyWatchlist
        return filters
    }

    @Watch('finalFilters', { deep: true })
    refreshData() {
        this.recentMatches = null
        this.nextLink = null
        this.lastIndex = 0
        this.selected = []
        this.loadMoreMatches()
    }

    loadMoreMatches() {
        this.loading = true
        apiClient.listMyRecentMatches({
            next: this.nextLink,
            start: this.lastIndex,
            end: this.lastIndex + maxTasksPerRequest,
            filters: this.finalFilters,
        }).then((resp : ApiData<HalResponse<RecentMatch[]>>) => {
            if (!this.recentMatches) {
                this.recentMatches = resp.data.data
            } else {
                this.recentMatches.push(...resp.data.data)
            }
            this.lastIndex += resp.data.data.length
            if ("next" in resp.data._links) {
                this.nextLink = resp.data._links["next"].href
            } else {
                this.nextLink = null
            }
        }).catch((err : any) => {
            console.log('Failed to list recent SquadOV matches: ' + err);
        }).finally(() => {
            this.loading = false
        })
    }

    deleteSelectedVods() {
        if (this.selected.length === 0) {
            return
        }

        this.deleteInProgress = true
        apiClient.deleteVods(this.selected).then(() => {
            this.refreshData()
            this.showHideDelete = false
        }).catch((err: any) => {
            console.log('Failed to delete VODs: ', err)
            this.showHideDeleteError = true
        }).finally(() => {
            this.deleteInProgress = false
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>