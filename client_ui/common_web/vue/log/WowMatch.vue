<template>
    <loading-container :is-loading="!currentMatch">
        <template v-slot:default="{ loading }">
            <v-container fluid v-if="!loading">
                <div class="d-flex align-center">
                    <wow-keystone-summary v-if="!!currentMatch.challenge"
                        :challenge="currentMatch.challenge"
                        :user-id="userId"
                        class="flex-grow-1 mb-4"
                    >
                    </wow-keystone-summary>

                    <wow-encounter-summary v-else-if="!!currentMatch.encounter"
                        :encounter="currentMatch.encounter"
                        :user-id="userId"
                        class="flex-grow-1 mb-4"
                    >
                    </wow-encounter-summary>

                    <wow-arena-summary v-else-if="!!currentMatch.arena"
                        :arena="currentMatch.arena"
                        :user-id="userId"
                        class="flex-grow-1 mb-4"
                    >
                    </wow-arena-summary>

                    <match-share-button
                        :match-uuid="matchUuid"
                        :game="SquadOvGames.WorldOfWarcraft"
                        :user-id="userId"
                    >
                    </match-share-button>
                </div>

                <v-row class="my-2" no-gutters>
                    <v-col :cols="theaterMode ? 12 : 8">
                        <video-player
                            ref="player"
                            :vod="vod"
                            :player-height.sync="currentPlayerHeight"
                            id="match-vod"
                            @toggle-theater-mode="theaterMode = !theaterMode"
                            :current-time.sync="vodTime"
                            :ready.sync="vodReady"
                        >
                        </video-player>

                        <wow-vod-pov-picker
                            :vod.sync="vod"
                            :match-uuid="matchUuid"
                            :user-id="userId"
                            :match-characters ="matchCharacters"
                            :character-associations="characterAssociations"
                            :timestamp="vodTime"
                        >
                        </wow-vod-pov-picker>
                    </v-col>

                    <v-col v-if="!theaterMode" cols="4">
                        <wow-match-events
                            :events="events"
                            :style="roundEventsStyle"
                            :has-vod="!!vod && vodReady"
                            :current-character="currentCharacter"
                            :start-time="startTime"
                            :sync-unified-events.sync="filteredEvents"
                            :match-characters="matchCharacters"
                            :selected-encounter.sync="selectedEncounter"
                            :is-arena="!!currentMatch.arena"
                            :use-teams="!!currentMatch.arena"
                            :friendly-team="friendlyTeam"
                            @go-to-event="goToVodTime"
                        >
                        </wow-match-events>
                    </v-col>
                </v-row>

                <v-row>
                    <wow-timeline
                        :user-id="userId"
                        :match-uuid="matchUuid"
                        :start-time="startTime"
                        :encounter-start-time="encounterStartTime"
                        :encounter-end-time="encounterEndTime"
                        :events="events"
                        :unified-events="filteredEvents"
                        :match-characters="matchCharacters"
                        :current-time="vodTime"
                        @go-to-time="goToVodTime"
                    >
                    </wow-timeline>
                </v-row>
            </v-container>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { GenericWowMatchContainer, getOppositeWowArenaTeam } from '@client/js/wow/matches'
import { VodAssociation } from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import { SerializedWowMatchEvents, UnifiedWowEventContainer, WowEncounter } from '@client/js/wow/events'
import {
    WowCharacter,
    WoWCharacterUserAssociation
} from '@client/js/wow/character'
import { SquadOvGames } from '@client/js/squadov/game'
import { getActiveUserId } from '@client/js/app'

import WowMatchEvents from '@client/vue/utility/wow/WowMatchEvents.vue'
import WowKeystoneSummary from '@client/vue/utility/wow/WowKeystoneSummary.vue'
import WowEncounterSummary from '@client/vue/utility/wow/WowEncounterSummary.vue'
import WowArenaSummary from '@client/vue/utility/wow/WowArenaSummary.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'
import WowTimeline from '@client/vue/utility/wow/WowTimeline.vue'
import WowVodPovPicker from '@client/vue/utility/wow/WowVodPovPicker.vue'
import MatchShareButton from '@client/vue/utility/squadov/MatchShareButton.vue'

@Component({
    components: {
        WowMatchEvents,
        WowKeystoneSummary,
        WowEncounterSummary,
        WowArenaSummary,
        LoadingContainer,
        VideoPlayer,
        WowTimeline,
        WowVodPovPicker,
        MatchShareButton,
    }
})
export default class WowMatch extends Vue {
    SquadOvGames: any = SquadOvGames
    
    @Prop({type:Number, required: true})
    userId!: number

    @Prop({required: true})
    matchUuid!: string

    $refs!: {
        player: VideoPlayer
    }
    
    matchCharacters: WowCharacter[] = []
    characterAssociations: WoWCharacterUserAssociation[] = []

    currentMatch: GenericWowMatchContainer | null = null
    selectedEncounter: WowEncounter | null = null

    events: SerializedWowMatchEvents | null = null
    filteredEvents: UnifiedWowEventContainer[] = []

    vod: VodAssociation | null = null
    vodTime: Date | null = null
    theaterMode: boolean = false
    currentPlayerHeight : number = 0
    vodReady: boolean = false

    get friendlyTeam(): number {
        let winningTeamId = this.currentMatch?.arena?.winningTeamId
        if (winningTeamId === null || winningTeamId === undefined) {
            return 0
        }

        return !!this.currentMatch?.arena?.success ? winningTeamId : getOppositeWowArenaTeam(winningTeamId)
    }

    goToVodTime(tm : Date) {
        if (!this.vod) {
            return
        }
        
        let diffMs = tm.getTime() - this.vod.startTime.getTime()
        this.$refs.player.goToTimeMs(diffMs)
    }

    get currentCharacter(): string | undefined {
        return this.userIdToChar.get(this.userId)
    }

    get userIdToChar(): Map<number, string> {
        let ret = new Map()
        for (let assoc of this.characterAssociations) {
            ret.set(assoc.userId, assoc.guid)
        }
        return ret
    }

    get roundEventsStyle() : any {
        return {
            'height': `${this.currentPlayerHeight + 56}px`,
        }
    }

    get encounterStartTime(): Date {
        if (!!this.selectedEncounter) {
            return this.selectedEncounter.startTm
        } else if (!!this.currentMatch?.encounter) {
            return this.currentMatch.encounter.tm
        } else if (!!this.currentMatch?.challenge) {
            return this.currentMatch.challenge.tm
        } else if (!!this.currentMatch?.arena) {
            return this.currentMatch.arena.tm
        } else {
            return this.startTime
        }
    }

    get encounterEndTime(): Date {
        if (!!this.selectedEncounter) {
            return this.selectedEncounter.endTm
        } else if (!!this.currentMatch?.encounter?.finishTime) {
            return this.currentMatch.encounter.finishTime
        } else if (!!this.currentMatch?.challenge?.finishTime) {
            return this.currentMatch.challenge.finishTime
        } else if (!!this.currentMatch?.arena?.finishTime) {
            return this.currentMatch.arena.finishTime
        } else {
            return this.endTime
        }
    }

    get startTime(): Date {
        if (!this.currentMatch) {
            return new Date()
        } else if (!!this.vod) {
            return this.vod.startTime
        } else if (!!this.currentMatch.encounter) {
            return this.currentMatch.encounter.tm
        } else if (!!this.currentMatch.challenge) {
            return this.currentMatch.challenge.tm
        } else if (!!this.currentMatch.arena) {
            return this.currentMatch.arena.tm
        } else {
            return new Date()
        }
    }

    get endTime(): Date {
        if (!this.currentMatch) {
            return new Date()
        } else if (!!this.vod) {
            return this.vod.endTime
        } else if (!!this.currentMatch.encounter?.finishTime) {
            return this.currentMatch.encounter.finishTime
        } else if (!!this.currentMatch.challenge?.finishTime) {
            return this.currentMatch.challenge.finishTime
        } else if (!!this.currentMatch.arena?.finishTime) {
            return this.currentMatch.arena.finishTime
        } else {
            return new Date()
        }
    }

    @Watch('matchUuid')
    @Watch('userId')
    refreshMatch() {
        this.currentMatch = null
        apiClient.getWoWMatch(this.userId, this.matchUuid).then((resp: ApiData<GenericWowMatchContainer>) => {
            this.currentMatch = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain WoW match: ', err)
        })
    }

    @Watch('matchUuid')
    @Watch('userId')
    refreshCharacters() {
        this.matchCharacters = []
        apiClient.listWoWCharactersForMatch(this.matchUuid, this.userId).then((resp: ApiData<WowCharacter[]>) => {
            this.matchCharacters = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain WoW match characters: ', err)
        })
    }

    @Watch('matchUuid')
    refreshCharacterAssociations() {
        this.characterAssociations = []
        apiClient.listWoWMatchCharacterAssociations(getActiveUserId(), this.matchUuid).then((resp: ApiData<WoWCharacterUserAssociation[]>) => {
            this.characterAssociations = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain WoW character associations: ', err)
        })
    }

    @Watch('matchUuid')
    @Watch('userId')
    refreshEvents() {
        this.events = null
        apiClient.getWoWMatchEvents(this.userId, this.matchUuid).then((resp: ApiData<SerializedWowMatchEvents>) => {
            this.events = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain WoW match events: ', err)
        })
    }

    mounted() {
        this.refreshMatch()
        this.refreshCharacters()
        this.refreshCharacterAssociations()
        this.refreshEvents()
    }
}

</script>

<style scoped>

#match-vod {
    width: 100%;
}

</style>