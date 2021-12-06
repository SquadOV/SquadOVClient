<template>
    <v-sheet class="full-parent-height">
        <div class="d-flex align-center pa-4">
            Events
            <v-spacer></v-spacer>

            <v-select
                class="item-select"
                v-model="participants"
                dense
                solo
                label="Player Filter"
                hide-details
                :items="selectOptions"
                multiple
            >
                <template v-slot:item="{item}">
                    <div class="d-flex align-center">
                        <v-checkbox
                            class="selection-checkbox"
                            dense
                            hide-details
                            :input-value="participants.includes(item.value)"
                            readonly
                        >
                        </v-checkbox>

                        <lol-participant-display
                            :participant="item.player"
                            :match="match"
                            :width-height="32"
                            :current-participant-id="currentParticipantId"
                            no-tooltip
                        >
                        </lol-participant-display>
                    </div>
                </template>

                <template v-slot:selection="{item}">
                    <div class="mx-1">
                        <lol-participant-display
                            :participant="item.player"
                            :match="match"
                            :width-height="32"
                            :current-participant-id="currentParticipantId"
                            no-tooltip
                        >
                        </lol-participant-display>
                    </div>
                </template>
            </v-select>

            <v-menu
                v-model="showFilters"
                bottom
                offset-y
                :close-on-content-click="false"
            >
                <template v-slot:activator="{on, attrs}">
                    <v-btn
                        v-on="on"
                        v-bind="attrs"
                        small
                        icon
                    >
                        <v-icon>
                            mdi-cog
                        </v-icon>
                    </v-btn>
                </template>

                <v-list dense>
                    <v-list-item @click="showKills = !showKills">
                        <v-list-item-action>
                            <v-checkbox v-model="showKills" @click.stop></v-checkbox>
                        </v-list-item-action>

                        <v-list-item-content>
                            <v-list-item-title>Kills</v-list-item-title>
                        </v-list-item-content>
                    </v-list-item>

                    <v-list-item @click="showWards = !showWards">
                        <v-list-item-action>
                            <v-checkbox v-model="showWards" @click.stop></v-checkbox>
                        </v-list-item-action>

                        <v-list-item-content>
                            <v-list-item-title>Wards</v-list-item-title>
                        </v-list-item-content>
                    </v-list-item>

                    <v-list-item @click="showTowers = !showTowers">
                        <v-list-item-action>
                            <v-checkbox v-model="showTowers" @click.stop></v-checkbox>
                        </v-list-item-action>

                        <v-list-item-content>
                            <v-list-item-title>Towers</v-list-item-title>
                        </v-list-item-content>
                    </v-list-item>

                    <v-list-item @click="showInhibitors = !showInhibitors">
                        <v-list-item-action>
                            <v-checkbox v-model="showInhibitors" @click.stop></v-checkbox>
                        </v-list-item-action>

                        <v-list-item-content>
                            <v-list-item-title>Inhibitors</v-list-item-title>
                        </v-list-item-content>
                    </v-list-item>

                    <v-list-item @click="showDragons = !showDragons">
                        <v-list-item-action>
                            <v-checkbox v-model="showDragons" @click.stop></v-checkbox>
                        </v-list-item-action>

                        <v-list-item-content>
                            <v-list-item-title>Dragons</v-list-item-title>
                        </v-list-item-content>
                    </v-list-item>

                    <v-list-item @click="showBarons = !showBarons">
                        <v-list-item-action>
                            <v-checkbox v-model="showBarons" @click.stop></v-checkbox>
                        </v-list-item-action>

                        <v-list-item-content>
                            <v-list-item-title>Barons</v-list-item-title>
                        </v-list-item-content>
                    </v-list-item>

                    <v-list-item @click="showRiftHerald = !showRiftHerald">
                        <v-list-item-action>
                            <v-checkbox v-model="showRiftHerald" @click.stop></v-checkbox>
                        </v-list-item-action>

                        <v-list-item-content>
                            <v-list-item-title>Rift Herald</v-list-item-title>
                        </v-list-item-content>
                    </v-list-item>
                </v-list>
            </v-menu>
        </div>
        <v-divider></v-divider>

        <v-list dense class="event-list pa-0">
            <v-list-item
                v-for="(eve, index) in filteredEvents"
                :key="`event-${eve.timestamp}-${index}`"
                :style="eventStyling(eve)"
            >
                <v-list-item-content class="event-time">
                    {{ formatRoundTime(eve.timestamp) }}
                </v-list-item-content>

                <v-list-item-content>
                    <lol-event-display
                        :match="match"
                        :timeline="timeline"
                        :input-event="eve"
                        :current-participant-id="currentParticipantId"
                    >
                    </lol-event-display>
                </v-list-item-content>

                <v-list-item-action class="ml-0" v-if="!!hasVod">
                    <v-btn
                        outlined
                        fab
                        small
                        color="white"
                        @click="goToEvent(eve)"
                    >
                        <v-icon>mdi-play</v-icon>
                    </v-btn>
                </v-list-item-action>
            </v-list-item>
        </v-list>
    </v-sheet>
</template>

<script lang="ts">

import Component, { mixins } from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import {
    LolMatch,
    getTeamIdFromParticipantId,
    extractSameTeamPlayersFromParticipantId,
    extractEnemyTeamPlayersFromParticipantId,
    extractSameTeamPlayersFromTeamId,
} from '@client/js/lol/matches'
import { LolMatchTimeline, LolMatchEvent, LolMatchFrame } from '@client/js/lol/timeline'
import { formatRoundTime } from '@client/js/valorant/valorant_utility'
import { computeColorForLolEvent } from '@client/js/lol/color'
import { LolParticipant } from '@client/js/lol/participant'
import LolEventDisplay from '@client/vue/utility/lol/LolEventDisplay.vue'
import CommonComponent from '@client/vue/CommonComponent'
import LolParticipantDisplay from '@client/vue/utility/lol/LolParticipantDisplay.vue'

@Component({
    components: {
        LolEventDisplay,
        LolParticipantDisplay,
    }
})
export default class LolEventManager extends mixins(CommonComponent) {
    formatRoundTime = formatRoundTime

    @Prop({required: true})
    match!: LolMatch

    @Prop({required: true})
    timeline!: LolMatchTimeline

    @Prop({required: true})
    currentParticipantId!: number | null | undefined

    @Prop({type: Array, required: true})
    displayEvents!: LolMatchEvent[]

    @Prop({type: Boolean, required: true})
    hasVod!: boolean

    showFilters: boolean = false
    // Event Filters
    showKills: boolean = true
    showWards: boolean = false
    showTowers: boolean = true
    showInhibitors: boolean = true
    showDragons: boolean = true
    showBarons: boolean = true
    showRiftHerald: boolean = true
    participants: number[] = []

    get currentTeamId(): number | undefined {
        if (!this.currentParticipantId) {
            return undefined
        }
        return getTeamIdFromParticipantId(this.match, this.currentParticipantId)
    }

    get selectOptions(): any[] {
        let allPlayers: LolParticipant[] = []
        if (!!this.currentParticipantId && !!this.currentTeamId) {
            allPlayers = [
                ...extractSameTeamPlayersFromParticipantId(this.match, this.currentParticipantId)!,
                ...extractEnemyTeamPlayersFromParticipantId(this.match, this.currentParticipantId)!
            ]
        } else {
            allPlayers = [
                ...extractSameTeamPlayersFromTeamId(this.match, 100),
                ...extractSameTeamPlayersFromTeamId(this.match, 200),
            ]
        }
        return allPlayers.map((ele: LolParticipant) => {
            return {
                player: ele,
                text: ele.summonerName,
                value: ele.participantId
            }
        })
    }

    goToEvent(e: LolMatchEvent) {
        this.sendAnalyticsEvent(this.AnalyticsCategory.MatchInfo, this.AnalyticsAction.GoToEvent, e.type, e.timestamp)
        this.$emit('go-to-timestamp', e.timestamp)
    }

    eventStyling(e : LolMatchEvent) : any {
        let borderHighlightColor: string = computeColorForLolEvent(this.match, e, this.currentParticipantId)
        let style: any = {
            'border-left': `5px solid var(--${borderHighlightColor})`
        }
        return style
    }

    get filteredEvents(): LolMatchEvent[] {
        let retEvents = this.timeline.info.frames.map((frame: LolMatchFrame) => {
            return frame.events.filter((ev: LolMatchEvent) => {
                return ((ev.type === 'CHAMPION_KILL' && this.showKills)
                    || ((ev.type === 'WARD_PLACED' || ev.type === 'WARD_KILL') && ev.wardType !== 'UNDEFINED' && ev.wardType !== 'TEEMO_MUSHROOM' && this.showWards)
                    || (ev.type === 'BUILDING_KILL' && ev.buildingType === 'TOWER_BUILDING' && this.showTowers)
                    || (ev.type === 'BUILDING_KILL' && ev.buildingType === 'INHIBITOR_BUILDING' && this.showInhibitors)
                    || (ev.type === 'ELITE_MONSTER_KILL' && ev.monsterType === 'DRAGON' && this.showDragons)
                    || (ev.type === 'ELITE_MONSTER_KILL' && ev.monsterType === 'BARON_NASHOR' && this.showBarons)
                    || (ev.type === 'ELITE_MONSTER_KILL' && ev.monsterType === 'RIFTHERALD' && !!ev.killerId && this.showRiftHerald))
                    && (this.participants.length === 0
                        || (ev.killerId !== null && this.participants.includes(ev.killerId))
                        || (ev.victimId !== null && this.participants.includes(ev.victimId))
                        || (ev.creatorId !== null && this.participants.includes(ev.creatorId))
                        || (ev.participantId !== null && this.participants.includes(ev.participantId))
                        || (ev.assistingParticipantIds !== null && ev.assistingParticipantIds.length > 0 && ev.assistingParticipantIds.some((id: number) => this.participants.includes(id)))
                    )
            })
        }).flat()

        retEvents.sort((a: LolMatchEvent, b: LolMatchEvent) => {
            return a.timestamp - b.timestamp
        })

        return retEvents
    }

    @Watch('filteredEvents')
    syncDisplayedEvents() {
        this.$emit('update:displayEvents', this.filteredEvents)
    }

    mounted() {
        this.syncDisplayedEvents()
    }
}

</script>

<style scoped>

.event-list {
    overflow-y: auto;
    height: calc(100% - 73px);
}

.event-time {
    flex: 0 0 auto !important;
}

</style>