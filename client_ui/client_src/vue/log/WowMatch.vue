<template>
    <loading-container :is-loading="!currentMatch">
        <template v-slot:default="{ loading }">
            <v-container fluid v-if="!loading">
                <v-row>
                    <v-col cols="12">
                        <wow-keystone-summary v-if="!!currentMatch.challenge"
                            :challenge="currentMatch.challenge"
                            :user-id="userId"
                            class="full-width"
                        >
                        </wow-keystone-summary>

                        <wow-encounter-summary v-else-if="!!currentMatch.encounter"
                            :encounter="currentMatch.encounter"
                            :user-id="userId"
                            class="full-width"
                        >
                        </wow-encounter-summary>
                    </v-col>
                </v-row>

                <v-row no-gutters>
                    <v-col :cols="theaterMode ? 12 : 8">
                        <video-player
                            ref="player"
                            :vod="vod"
                            :player-height.sync="currentPlayerHeight"
                            id="match-vod"
                            @toggle-theater-mode="theaterMode = !theaterMode"
                            :current-time.sync="vodTime"
                        >
                        </video-player>
                    </v-col>

                    <v-col v-if="!theaterMode" cols="4">
                        <wow-match-events
                            :events="events"
                            :style="roundEventsStyle"
                            :has-vod="!!vod"
                            :current-character="currentCharacter"
                            :start-time="startTime"
                            :sync-unified-events.sync="filteredEvents"
                            :match-characters="matchCharacters"
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
                        :end-time="endTime"
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
import { GenericWowMatchContainer } from '@client/js/wow/matches'
import { VodAssociation } from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import { SerializedWowMatchEvents, UnifiedWowEventContainer } from '@client/js/wow/events'
import {
    WowCharacter,
    WoWCharacterUserAssociation
} from '@client/js/wow/character'

import WowMatchEvents from '@client/vue/utility/wow/WowMatchEvents.vue'
import WowKeystoneSummary from '@client/vue/utility/wow/WowKeystoneSummary.vue'
import WowEncounterSummary from '@client/vue/utility/wow/WowEncounterSummary.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'
import WowTimeline from '@client/vue/utility/wow/WowTimeline.vue'

@Component({
    components: {
        WowMatchEvents,
        WowKeystoneSummary,
        WowEncounterSummary,
        LoadingContainer,
        VideoPlayer,
        WowTimeline
    }
})
export default class WowMatch extends Vue {
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

    events: SerializedWowMatchEvents | null = null
    filteredEvents: UnifiedWowEventContainer[] = []

    vod: VodAssociation | null = null
    vodTime: Date | null = null
    theaterMode: boolean = false
    currentPlayerHeight : number = 0

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
            'height': `${this.currentPlayerHeight}px`,
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
        } else {
            return new Date()
        }
    }

    get endTime(): Date {
        if (!this.currentMatch) {
            return new Date()
        } else if (!!this.vod) {
            return this.vod.endTime
        } else if (!!this.currentMatch.encounter && !!this.currentMatch.encounter.finishTime) {
            return this.currentMatch.encounter.finishTime
        } else if (!!this.currentMatch.challenge && !!this.currentMatch.challenge.finishTime) {
            return this.currentMatch.challenge.finishTime
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
        apiClient.listWoWMatchCharacterAssociations(this.$store.state.currentUser.id, this.matchUuid).then((resp: ApiData<WoWCharacterUserAssociation[]>) => {
            this.characterAssociations = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain WoW character associations: ', err)
        })
    }

    @Watch('matchUuid')
    @Watch('userId')
    refreshVod() {
        this.vod = null
        apiClient.findVodFromMatchUserId(this.matchUuid, this.userId).then((resp : ApiData<VodAssociation>) => {
            this.vod = resp.data
        }).catch((err : any) => {
            this.vod = null
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
        this.refreshVod()
    }
}

</script>

<style scoped>

#match-vod {
    width: 100%;
}

</style>