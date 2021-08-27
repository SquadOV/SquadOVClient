<template>
    <div :style="playtimeWindowStyle">
        <div class="d-flex align-center playtime-header pa-1">
            <v-icon class="mr-1">
                mdi-clock-outline
            </v-icon>
            <span class="text-subtitle-1 font-weight-bold">Recorded Playtime (7 Days)</span>
            <v-spacer></v-spacer>
            <v-btn
                @click="refreshData"
                icon
                x-small
            >
                <v-icon>
                    mdi-refresh
                </v-icon>
            </v-btn>
        </div>
        <v-divider></v-divider>
        <loading-container :is-loading="!playtime">
            <template v-slot:default="{ loading }">
                <v-list dense v-if="!loading">
                    <v-list-item
                        class="d-flex align-center"
                        v-for="(pt, idx) in displayPlaytime"
                        :key="idx"
                    >
                        <v-list-item-content>
                             <v-img
                                width="24px"
                                max-width="24px"
                                height="24px"
                                max-height="24px"
                                contain
                                :src="$root.generateAssetUri(gameToIcon(pt.game))"
                            >
                            </v-img>
                        </v-list-item-content>

                        <v-list-item-content>
                            <v-list-item-title>{{ displayedPlaytime(pt.timeMs) }}</v-list-item-title>
                            <v-list-item-subtitle>{{ gameToName(pt.game) }}</v-list-item-subtitle>
                        </v-list-item-content>
                    </v-list-item>

                    <div class="refresh-text long-text px-2 pt-2">Last refreshed {{ minutesSinceLastRefresh }} ago. Next refresh in {{ minutesToNextRefresh }}.</div>
                </v-list>

                
            </template>
        </loading-container>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { TotalRecordedPlaytime } from '@client/js/squadov/playtime'
import { SquadOvGames, gameToIcon, gameToName } from '@client/js/squadov/game'

const REFRESH_FREQUENCY_MS: number = 30 * 60 * 1000
const REFRESH_INTERVAL_MS: number = 60 * 1000
const QUERY_TIME: number = 7 * 24 * 60 * 60

interface PlaytimeDatum {
    game: SquadOvGames
    timeMs: number
}

@Component({
    components: {
        LoadingContainer
    }
})
export default class TotalRecordedPlaytimeWidget extends Vue {
    gameToIcon = gameToIcon
    gameToName = gameToName
    playtime: TotalRecordedPlaytime | null = null
    lastRefresh: Date | null = null
    minutesSinceLastRefresh: string = ''
    msToNextRefresh: number = REFRESH_FREQUENCY_MS
    itvl: number = 0

    updateMinutesSinceLastRefresh() {
        if (!this.lastRefresh) {
            this.minutesSinceLastRefresh = '0 minutes'
        } else {
            let elapsed = Math.round((new Date().getTime() - this.lastRefresh.getTime()) / (60 * 1000))
            if (elapsed == 1) {
                this.minutesSinceLastRefresh = '1 minute'
            } else {
                this.minutesSinceLastRefresh = `${elapsed} minutes`
            }
        }
    }

    displayedPlaytime(ms: number): string {
        let elapsed = ms / (60 * 60 * 1000)
        if (elapsed < 1) {
            return `${(elapsed * 60).toFixed(1)} minutes`
        } else {
            return `${elapsed.toFixed(1)} hours`
        }
    }

    get minutesToNextRefresh(): string {
        let elapsed = Math.round(this.msToNextRefresh / (60 * 1000))
        if (elapsed == 1) {
            return '1 minute'
        } else {
            return `${elapsed} minutes`
        }
    }

    get displayPlaytime(): PlaytimeDatum[] {
        if (!this.playtime) {
            return []
        }

        let ret: PlaytimeDatum[] = []
        if (this.playtime.aimlabMs > 0) {
            ret.push({
                game: SquadOvGames.AimLab,
                timeMs: this.playtime.aimlabMs,
            })
        }

        if (this.playtime.csgoMs > 0) {
            ret.push({
                game: SquadOvGames.Csgo,
                timeMs: this.playtime.csgoMs,
            })
        }

        if (this.playtime.hearthstoneMs > 0) {
            ret.push({
                game: SquadOvGames.Hearthstone,
                timeMs: this.playtime.hearthstoneMs,
            })
        }

        if (this.playtime.lolMs > 0) {
            ret.push({
                game: SquadOvGames.LeagueOfLegends,
                timeMs: this.playtime.lolMs,
            })
        }

        if (this.playtime.tftMs > 0) {
            ret.push({
                game: SquadOvGames.TeamfightTactics,
                timeMs: this.playtime.tftMs,
            })
        }

        if (this.playtime.valorantMs > 0) {
            ret.push({
                game: SquadOvGames.Valorant,
                timeMs: this.playtime.valorantMs,
            })
        }

        if (this.playtime.wowMs > 0) {
            ret.push({
                game: SquadOvGames.WorldOfWarcraft,
                timeMs: this.playtime.wowMs,
            })
        }

        ret.sort((a: PlaytimeDatum, b: PlaytimeDatum) => {
            return b.timeMs - a.timeMs
        })

        return ret
    }

    @Watch('$store.state.currentUser.id')
    refreshData() {
        apiClient.getRecentPlaytime(QUERY_TIME).then((resp: ApiData<TotalRecordedPlaytime>) => {
            this.playtime = resp.data
            this.lastRefresh = new Date()
            this.msToNextRefresh = REFRESH_FREQUENCY_MS
            this.updateMinutesSinceLastRefresh()
        }).catch((err: any) => {
            console.error('Failed to obtain recent playtime: ', err)
        })
    }

    startRefreshLoop() {
        this.itvl = window.setInterval(() => {
            this.msToNextRefresh -= REFRESH_INTERVAL_MS
            if (this.msToNextRefresh <= 0) {
                this.refreshData()
            }
            this.updateMinutesSinceLastRefresh()
        }, REFRESH_INTERVAL_MS)
    }

    beforeDestroy() {
        if (!!this.itvl) {
            window.clearInterval(this.itvl)
        }
    }

    mounted () {
        this.refreshData()
        this.startRefreshLoop()
    }

    get playtimeWindowStyle(): any {        
        return {
        }
    }
}

</script>

<style scoped>

.playtime-header {
    background-color: rgba(255, 255, 255, 0.1);
}

.refresh-text {
    font-size: 0.6rem;
    font-weight: 500;
    letter-spacing: 0.16667em !important;
    font-family: "Roboto", sans-serif !important;
    text-transform: uppercase !important;
}

</style>