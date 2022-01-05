<template>
    <loading-container :is-loading="!loaded">
        <template v-slot:default="{ loading }">
            <v-btn
                v-if="collapseSidePanel"
                id="reshow-sidebar"
                @click="collapseSidePanel = false"
                small
            >
                <v-icon>
                    mdi-chevron-triple-right
                </v-icon>
            </v-btn>

            <v-container fluid v-if="!loading">
                <v-row>
                    <v-col cols="3" v-if="!collapseSidePanel">
                        <status-display class="mb-4"></status-display>

                        <!-- User and squad -->
                        <div class="d-flex align-center">
                            <div class="text-h6 font-weight-bold">
                                Welcome back, {{ $store.state.currentUser.username }}!
                            </div>

                            <v-btn
                                @click="collapseSidePanel = true"
                                small
                                icon
                            >
                                <v-icon>
                                    mdi-chevron-triple-left
                                </v-icon>
                            </v-btn>
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
                            <v-dialog max-width="40%" v-model="showHideInviteSquad">
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
                                    @on-send-invite="showHideInviteSquad = false"
                                >
                                </squad-invite-create-card>
                            </v-dialog>
                        </div>

                        <!-- recommend squads -->
                        <div
                            id="rec-squads"
                            class="my-2"
                            v-if="!!recommendedSquads && recommendedSquads.length > 0"
                        >
                            <div class="ma-2 text-h6 font-weight-bold">Recommended Squads!</div>
                            <v-divider></v-divider>
                            <v-list-item
                                v-for="(squad, idx) in recommendedSquads"
                                :key="`squad-${idx}`"
                            >
                                <v-list-item-content>
                                    <v-list-item-title class="long-text">{{ squad.squadName }}</v-list-item-title>
                                </v-list-item-content>

                                <v-list-item-action>
                                    <v-btn color="primary" @click="joinPublicSquad(squad.id)" :loading="joiningPublicSquad">
                                        Join
                                    </v-btn>
                                </v-list-item-action>
                            </v-list-item>
                        </div>

                        <!-- Squad member list and status -->
                        <v-data-table
                            class="squad-member-list"
                            v-if="filteredSquadMembers.length > 0"
                            :headers="squadTableHeaders"
                            :items="squadTableItems"
                            :items-per-page="USERS_PER_PAGE"
                            :page="userPage"
                            hide-default-footer
                            hide-default-header
                            @click:row="goToUserProfile(arguments[0].userId)"
                        >
                            <template v-slot:item.status="{ item }">
                                <user-activity-display
                                    :status="item.status"
                                >
                                </user-activity-display>
                            </template>

                            <template v-slot:footer="{props}">
                                <v-pagination
                                    v-if="props.pagination.itemsLength > props.pagination.itemsPerPage"
                                    :length="Math.ceil(props.pagination.itemsLength / props.pagination.itemsPerPage)"
                                    v-model="userPage"
                                >
                                </v-pagination>
                            </template>
                        </v-data-table>

                        <div class="d-flex justify-center long-text text-subtitle-1" v-else>Nobody's here just yet. Invite your friends!</div>

                        <div class="mt-2">
                            <div class="text-overline">
                                Invite your friends to SquadOV!
                            </div>
                            <referral-link></referral-link>
                        </div>
                    </v-col>

                    <v-col :cols="collapseSidePanel ? 12 : 9">
                        <!-- Recent recorded games -->
                        <recent-recorded-matches
                            ref="recent"
                            disable-mini
                        >
                        </recent-recorded-matches>
                    </v-col>
                </v-row>
            </v-container>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Component, {mixins} from 'vue-class-component'
import CommonComponent from '@client/vue/CommonComponent'
import { Watch } from 'vue-property-decorator'
import * as pi from '@client/js/pages'
import { Squad, SquadMembership } from '@client/js/squadov/squad'
import { apiClient, ApiData } from '@client/js/api'
import { SquadOvActivity, TrackedUserStatsManager, TrackedUserStatus } from '@client/js/squadov/status'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import StatusDisplay from '@client/vue/utility/squadov/StatusDisplay.vue'
import NewsDisplay from '@client/vue/utility/squadov/NewsDisplay.vue'
import SquadCreationCard from '@client/vue/utility/squads/SquadCreationCard.vue'
import SquadInviteCreateCard from '@client/vue/utility/squads/SquadInviteCreateCard.vue'
import UserActivityDisplay from '@client/vue/utility/squadov/UserActivityDisplay.vue'
import TotalRecordedPlaytimeWidget from '@client/vue/utility/squadov/TotalRecordedPlaytimeWidget.vue'
import RecentRecordedMatches from '@client/vue/log/RecentRecordedMatches.vue'
import ReferralLink from '@client/vue/utility/squadov/ReferralLink.vue'
import { compareString, compareUserStatus } from '@client/js/cmp'

const LARGE_SQUAD_THRESHOLD: number = 50
const USERS_PER_PAGE: number = 10

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
        ReferralLink,
    }
})
export default class Dashboard extends mixins(CommonComponent) {
    USERS_PER_PAGE = USERS_PER_PAGE
    preselectedSquad: string | null = null
    mySquads: SquadMembership[] = []
    selectedSquad: SquadMembership | undefined | null = null
    squadMembers: SquadMembership[] = []
    showHideCreateSquad: boolean = false
    showHideInviteSquad: boolean = false
    recommendedSquads: Squad[] | null = null
    userPage: number = 1
    joiningPublicSquad: boolean = false
    collapseSidePanel: boolean = false

    $refs!: {
        recent: RecentRecordedMatches
    }

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
                userId: a.userId,
            }
        })
    }

    get squadItems(): any[] {
        return this.mySquads.map((ele: SquadMembership) => {
            return {
                text: `${ele.squad.squadName}`,
                value: ele,
            }
        })
    }

    get filteredSquadMembers(): SquadMembership[] {
        return this.squadMembers
            .filter((ele: SquadMembership) => ele.userId !== this.$store.state.currentUser.id)
            .filter((ele: SquadMembership) => {
                if (this.squadMembers.length > LARGE_SQUAD_THRESHOLD) {
                    let status = this.$store.state.status.status[ele.userId]
                    if (status === undefined) {
                        return false
                    }
                    return status.activity != SquadOvActivity.Offline
                } else {
                    return true
                }
            })
            .sort((a: SquadMembership, b: SquadMembership) => {
                let aStatus: TrackedUserStatus | undefined = this.$store.state.status.status[a.userId]
                let bStatus: TrackedUserStatus | undefined = this.$store.state.status.status[b.userId]

                let aActivity = !!aStatus ? aStatus.activity : SquadOvActivity.Offline
                let bActivity = !!bStatus ? bStatus.activity : SquadOvActivity.Offline

                return compareUserStatus(aActivity, bActivity) || compareString(a.username, b.username)
            })
    }

    @Watch('squadMembers')
    resubscribeToStatus(newMembers: SquadMembership[], oldMembers: SquadMembership[]) {
        let newSet: Set<number> = new Set(newMembers.map((ele: SquadMembership) => ele.userId))
        let oldSet: Set<number> = new Set(oldMembers.map((ele: SquadMembership) => ele.userId))

        let unsubUsers = oldMembers.filter((ele: SquadMembership) => !newSet.has(ele.userId) && ele.userId !== this.$store.state.currentUser.id).map((ele: SquadMembership) => ele.userId)
        let subUsers = newMembers.filter((ele: SquadMembership) => !oldSet.has(ele.userId) && ele.userId !== this.$store.state.currentUser.id).map((ele: SquadMembership) => ele.userId)

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
                console.error('Failed to get all squad memberships: ', err)
            })
        }
    }

    refreshSquads() {
        apiClient.getUserSquads(this.$store.state.currentUser.id).then((resp: ApiData<SquadMembership[]>) => {
            this.mySquads = resp.data
        }).catch((err: any) => {
            console.error('Failed to obtain user squads: ', err)
        })

        apiClient.getUserRecommendedSquads().then((resp: ApiData<Squad[]>) => {
            this.recommendedSquads = resp.data
        }).catch((err: any) => {
            console.error('Failed to obtain user recommended squads: ', err)
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
        this.redirectToSetupWizardOnFirstTime()
        this.refreshSquads()
    }

    onNewSquad(squadName: string) {
        this.preselectedSquad = squadName
        this.showHideCreateSquad = false
        this.refreshSquads()
    }

    redirectToSetupWizardOnFirstTime() {
///#if !DESKTOP
        if (!localStorage.getItem('squadovSetup')) {
            this.$router.push({
                name: pi.SetupWizardPageId
            })
        }
///#endif
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

    joinPublicSquad(squadId: number) {
        this.joiningPublicSquad = true
        apiClient.joinPublicSquad(squadId).then(() => {
            if (!!this.recommendedSquads) {
                this.recommendedSquads = this.recommendedSquads.filter((ele: Squad) => ele.id !== squadId)
            }

            this.refreshSquads()
            this.$refs.recent.refreshData()
        }).catch((err: any) => {
            console.error('Failed to join public squad: ', err)
        }).finally(() => {
            this.joiningPublicSquad = false
        })
    }

    goToUserProfile(userId: number) {
        this.$router.push({
            name: pi.UserProfileIdPageId,
            params: {
                userId: `${userId}`,
            }
        })
    }
}

</script>

<style scoped>

.choice {
    margin-left: 8px;
    margin-right: 8px;
}

#rec-squads {
    border-radius: 4px;
    border: 1px solid yellow;
}

#reshow-sidebar {
    border: 1px solid white;
    border-radius: 0px 5px 5px 0px;
    position: absolute;
    top: calc(10% + 48px);
    left: -1px;
    z-index: 10;
}

.squad-member-list >>> tbody tr :hover {
    cursor: pointer;
}

</style>