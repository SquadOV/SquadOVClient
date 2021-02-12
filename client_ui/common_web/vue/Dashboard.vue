<template>
    <loading-container :is-loading="!loaded">
        <template v-slot:default="{ loading }">
            <v-container fluid v-if="!loading">
                <v-row>
                    <v-col cols="3">
                        <!-- User and squad -->
                        <div class="text-h6 font-weight-bold">
                            Welcome back, {{ $store.state.currentUser.username }}!
                        </div>
                        <v-divider class="mt-2 mb-4"></v-divider>

                        <!-- Dropdown with user squads -->
                        <div class="d-flex align-center mb-1">
                            <span class="text-caption font-weight-bold mr-2">Squads</span>
                            <v-select
                                class="ma-0 pa-0"
                                v-model="selectedSquad"
                                :items="squadItems"
                                label="Your Squads"
                                hide-details
                            >
                            </v-select>
                        </div>

                        <!-- Buttons to create a squad and invite friend to a squad -->
                        <div class="d-flex flex-wrap">
                            <v-dialog persistent max-width="60%" v-model="showHideCreateSquad">
                                <template v-slot:activator="{on, attrs}">
                                    <v-btn
                                        v-on="on"
                                        v-bind="attrs"
                                        small
                                        class="flex-grow-1"
                                        color="success"
                                    >
                                        New
                                    </v-btn>
                                </template>

                                <squad-creation-card
                                    @on-new-squad="onNewSquad"
                                    @cancel-new-squad="showHideCreateSquad = false"
                                >
                                </squad-creation-card>
                            </v-dialog>

                            <v-btn
                                :disabled="!selectedSquad"
                                :to="manageSquadTo"
                                color="secondary"
                                small
                                class="flex-grow-1 ml-2"
                            >
                                Manage
                            </v-btn>
                        </div>

                        <div class="mt-2" v-if="!!selectedSquad">
                            <v-dialog persistent max-width="60%" v-model="showHideInviteSquad">
                                <template v-slot:activator="{on, attrs}">
                                    <v-btn
                                        v-on="on"
                                        v-bind="attrs"
                                        block
                                        small
                                        color="primary"
                                    >
                                        Invite
                                    </v-btn>
                                </template>

                                <squad-invite-create-card
                                    :squad-id="selectedSquad.squad.id"
                                    @on-cancel-invite="showHideInviteSquad = false"
                                    @on-send-invite="showHideInviteSquad = false"
                                >
                                </squad-invite-create-card>
                            </v-dialog>
                        </div>

                        <!-- Squad member list and status -->
                        <v-data-table
                            v-if="filteredSquadMembers.length > 0"
                            :headers="squadTableHeaders"
                            :items="squadTableItems"
                            :items-per-page="filteredSquadMembers.length"
                            hide-default-footer
                            hide-default-header
                        >
                            <template v-slot:item.status="{ item }">
                                <user-activity-display
                                    :status="item.status"
                                >
                                </user-activity-display>
                            </template>
                        </v-data-table>

                        <div class="d-flex justify-center long-text text-subtitle-1" v-else>Nobody's here just yet. Invite your friends!</div>

                        <!-- Recent recorded play time -->
                        <total-recorded-playtime-widget class="mt-4"></total-recorded-playtime-widget>
                    </v-col>

                    <v-col cols="6">
                        <!-- Recent recorded games -->
                        <recent-recorded-matches></recent-recorded-matches>
                    </v-col>

                    <v-col cols="3">
                        <!-- News and updates -->
                        <status-display class="mb-4"></status-display>
                        <news-display></news-display>
                    </v-col>
                </v-row>
            </v-container>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import * as pi from '@client/js/pages'
import { SquadMembership } from '@client/js/squadov/squad'
import { apiClient, ApiData } from '@client/js/api'
import { TrackedUserStatsManager } from '@client/js/squadov/status'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import StatusDisplay from '@client/vue/utility/squadov/StatusDisplay.vue'
import NewsDisplay from '@client/vue/utility/squadov/NewsDisplay.vue'
import SquadCreationCard from '@client/vue/utility/squads/SquadCreationCard.vue'
import SquadInviteCreateCard from '@client/vue/utility/squads/SquadInviteCreateCard.vue'
import UserActivityDisplay from '@client/vue/utility/squadov/UserActivityDisplay.vue'
import TotalRecordedPlaytimeWidget from '@client/vue/utility/squadov/TotalRecordedPlaytimeWidget.vue'
import RecentRecordedMatches from '@client/vue/log/RecentRecordedMatches.vue'

@Component({
    components: {
        LoadingContainer,
        StatusDisplay,
        NewsDisplay,
        SquadCreationCard,
        SquadInviteCreateCard,
        UserActivityDisplay,
        TotalRecordedPlaytimeWidget,
        RecentRecordedMatches,
    }
})
export default class Dashboard extends Vue {
    preselectedSquad: string | null = null
    mySquads: SquadMembership[] = []
    selectedSquad: SquadMembership | undefined | null = null
    squadMembers: SquadMembership[] = []
    showHideCreateSquad: boolean = false
    showHideInviteSquad: boolean = false

    get squadTableHeaders(): any[] {
        return [
            {
                text: 'Status',
                value: 'status',
                width: '48px',
            },
            {
                text: 'Member',
                value: 'member'
            },
        ]
    }

    get squadTableItems(): any[] {
        return this.filteredSquadMembers.map((a: SquadMembership) => {
            return {
                status: this.$store.state.status.status[a.userId],
                member: a.username,
            }
        })
    }

    get squadItems(): any[] {
        return this.mySquads.map((ele: SquadMembership) => {
            return {
                text: `${ele.squad.squadName} (${ele.squad.squadGroup})`,
                value: ele,
            }
        })
    }

    get filteredSquadMembers(): SquadMembership[] {
        return this.squadMembers.filter((ele: SquadMembership) => ele.userId !== this.$store.state.currentUser.id)
    }

    @Watch('filteredSquadMembers')
    resubscribeToStatus(newMembers: SquadMembership[], oldMembers: SquadMembership[]) {
        let newSet: Set<number> = new Set(newMembers.map((ele: SquadMembership) => ele.userId))
        let oldSet: Set<number> = new Set(oldMembers.map((ele: SquadMembership) => ele.userId))

        let unsubUsers = oldMembers.filter((ele: SquadMembership) => !newSet.has(ele.userId)).map((ele: SquadMembership) => ele.userId)
        let subUsers = newMembers.filter((ele: SquadMembership) => !oldSet.has(ele.userId)).map((ele: SquadMembership) => ele.userId)

        //@ts-ignore
        let tracker: TrackedUserStatsManager = this.$root.statusTracker
        tracker.unsubscribeFrom(unsubUsers)
        tracker.subscribeTo(subUsers)
    }

    @Watch('mySquads')
    refreshSelectedSquad() {
        if (this.mySquads.length === 0) {
            this.selectedSquad = null
        } else if (!!this.preselectedSquad) {
            this.selectedSquad = this.mySquads.find((ele: SquadMembership) => ele.squad.squadName === this.preselectedSquad)
        } else {
            this.selectedSquad = this.mySquads[0]
        }
    }

    @Watch('selectedSquad')
    refreshSquadMembers() {
        if (!this.selectedSquad) {
            this.squadMembers = []
        } else {
            apiClient.getSquadUsers(this.selectedSquad.squad.id).then((resp: ApiData<SquadMembership[]>) => {
                this.squadMembers = resp.data
            }).catch((err: any) => {
                console.log('Failed to get all squad memberships: ', err)
            })
        }
    }

    refreshSquads() {
        apiClient.getUserSquads(this.$store.state.currentUser.id).then((resp: ApiData<SquadMembership[]>) => {
            this.mySquads = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain user squads: ', err)
        })
    }

    get loaded(): boolean {
        return !!this.$store.state.currentUser
    }

    get logsTo() : any {
        return {
            name: pi.LogPageId,
            params: {
                userId: this.$store.state.currentUser.id
            }
        }
    }

    get performanceTo() : any {
        return {
            name: pi.PerformancePageId,
            params: {
                userId: this.$store.state.currentUser.id
            }
        }
    }

    mounted() {
        this.refreshSquads()
    }

    onNewSquad(squadName: string) {
        this.preselectedSquad = squadName
        this.showHideCreateSquad = false
        this.refreshSquads()
    }

    get manageSquadTo(): any {
        if (!this.selectedSquad) {
            return {}
        }

        return {
            name: pi.SingleSquadPageId,
            params: {
                squadId: this.selectedSquad.squad.id
            }
        }
    }
}

</script>

<style scoped>

.choice {
    margin-left: 8px;
    margin-right: 8px;
}

</style>