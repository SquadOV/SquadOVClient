<template>
    <loading-container :is-loading="!recentMatches">
        <template v-slot:default="{ loading }">
            <template v-if="!loading && recentMatches.length > 0">
                <div class="d-flex text-h6 align-center">
                    Recent Games

                    <v-spacer></v-spacer>

                    <v-btn icon @click="refreshData">
                        <v-icon>
                            mdi-refresh
                        </v-icon>
                    </v-btn>
                </div>
                <v-divider class="my-2"></v-divider>

                <template v-for="(group, gidx) in groupedMatches">
                    <div class="d-flex align-center my-2" :key="`group-header-${gidx}`">
                        <v-divider class="mr-4"></v-divider>

                        <div class="text-caption">
                            Played {{ group.days }}
                        </div>
                    </div>

                    <div class="full-width" :key="`group-matches-${gidx}`">
                        <recent-match-display
                            class="mb-4 full-width"
                            v-for="(match, idx) in group.matches"
                            :key="idx"
                            :match="match"
                        >
                        </recent-match-display>
                    </div>
                </template>

                <v-btn
                    v-if="hasNext"
                    color="primary"
                    block
                    @click="loadMoreMatches"  
                >
                    Load More
                </v-btn>
            </template>

            <div class="d-flex justify-center align-center full-parent-height full-width long-text" v-else>
                <div class="text-h6">
                    No recently recorded matches found for you or your squadmates! Start playing some games to get SquadOV to automatically record VODs. Don't forget to invite your friends!
                </div>
            </div>
        </template>
    </loading-container>
</template>

<script lang="ts">

const maxTasksPerRequest : number = 10

import Vue from 'vue'
import Component from 'vue-class-component'
import { RecentMatch, checkRecentMatchValidity } from '@client/js/squadov/recentMatch'
import { apiClient, HalResponse, ApiData } from '@client/js/api'
import { numDaysAgo, numDaysAgoToString } from '@client/js/time'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import RecentMatchDisplay from '@client/vue/utility/RecentMatchDisplay.vue'

interface GroupedMatch {
    days: string
    matches: RecentMatch[]
}

@Component({
    components: {
        LoadingContainer,
        RecentMatchDisplay,
    }
})
export default class RecentRecordedMatches extends Vue {
    recentMatches: RecentMatch[] | null = null
    lastIndex: number = 0
    nextLink: string | null = null

    get hasNext() : boolean {
        return !!this.nextLink
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
        this.filteredMatches.forEach((ele: RecentMatch) => {
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

    refreshData() {
        this.recentMatches = null
        this.nextLink = null
        this.lastIndex = 0
        this.loadMoreMatches()
    }

    loadMoreMatches() {
        apiClient.listMyRecentMatches({
            next: this.nextLink,
            start: this.lastIndex,
            end: this.lastIndex + maxTasksPerRequest,
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
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>