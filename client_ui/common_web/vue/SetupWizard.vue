<template>
    <div class="d-flex flex-column full-parent-height pa-4">
        <v-stepper v-model="step" class="flex-grow-1" non-linear v-if="ready">
            <v-stepper-header>
                <v-stepper-step
                    :complete="step > 1"
                    :step="1"
                    editable
                >
                    Welcome
                </v-stepper-step>

                <v-stepper-step
                    v-for="(st, idx) in dynamicSteps"
                    :key="`header-${idx}`"
                    :complete="step > (idx + 2)"
                    :step="idx + 2"
                    editable
                >
                    {{ stepToString(st) }}
                </v-stepper-step>

                <v-stepper-step
                    :complete="false"
                    :step="totalSteps"
                    editable
                >
                    Finish
                </v-stepper-step>
            </v-stepper-header>

            <v-stepper-items>
                <v-stepper-content step="1">
                    <h1>Welcome to SquadOV!</h1>
                    <p>
                        Thanks for joining us, we're ecstatic to have you here!
                        This setup wizard will help you get started with recording your VODs complete with match history details ASAP!
                    </p>

                    <div class="d-flex flex-column justify-center align-center">
                        <div class="text-h6">
                            Select the games you want to setup SquadOV for (all Riot games will be selected if you select one of them)!
                        </div>

                        <v-item-group class="d-flex flex-wrap" multiple v-model="gameSteps">
                            <v-item v-slot="{active, toggle}" :value="DynamicSteps.Csgo">
                                <v-img
                                    :class="`game ${active ? 'chosen-game' : 'not-chosen-game'} mx-2`"
                                    width="256px"
                                    max-width="256px"
                                    :src="$root.generateAssetUri('assets/csgo_box.jpg')"
                                    contain
                                    @click="toggle"
                                >
                                </v-img>
                            </v-item>

                            <v-item v-slot="{active, toggle}" :value="DynamicSteps.Riot">
                                <v-img
                                    :class="`game ${active ? 'chosen-game' : 'not-chosen-game'} mx-2`"
                                    width="256px"
                                    max-width="256px"
                                    :src="$root.generateAssetUri('assets/lol_box.jpg')"
                                    contain
                                    @click="toggle"
                                >
                                </v-img>
                            </v-item>

                            <v-item v-slot="{active, toggle}" :value="DynamicSteps.Riot">
                                <v-img
                                    :class="`game ${active ? 'chosen-game' : 'not-chosen-game'} mx-2`"
                                    width="256px"
                                    max-width="256px"
                                    :src="$root.generateAssetUri('assets/tft_box.jpg')"
                                    contain
                                    @click="toggle"
                                >
                                </v-img>
                            </v-item>

                            <v-item v-slot="{active, toggle}" :value="DynamicSteps.Riot">
                                <v-img
                                    :class="`game ${active ? 'chosen-game' : 'not-chosen-game'} mx-2`"
                                    width="256px"
                                    max-width="256px"
                                    :src="$root.generateAssetUri('assets/valorant_box.jpg')"
                                    contain
                                    @click="toggle"
                                >
                                </v-img>
                            </v-item>

                            <v-item v-slot="{active, toggle}" :value="DynamicSteps.Wow">
                                <v-img
                                    :class="`game ${active ? 'chosen-game' : 'not-chosen-game'} mx-2`"
                                    width="256px"
                                    max-width="256px"
                                    :src="$root.generateAssetUri('assets/wow_box.jpg')"
                                    contain
                                    @click="toggle"
                                >
                                </v-img>
                            </v-item>
                        </v-item-group>

                        <div class="text-subtitle-2">
                            If you don't see a supported game here, it means that SquadOV does not require any additional setup to record it!
                        </div>
                    </div>
                </v-stepper-content>

                <v-stepper-content
                    v-for="(st, idx) in dynamicSteps"
                    :key="`content-${idx}`"
                    :step="idx + 2"
                >
                    <template v-if="st == DynamicSteps.Riot">
                        SquadOV uses Riot's official API to obtain details about your matches.
                        To best ensure your privacy, we require that all users link their Riot account to SquadOV before we automatically record matches for any Riot game.

                        <div class="d-flex justify-center mt-4">
                            <riot-account-chooser
                                v-if="!!riotAccounts"
                                v-model="selectedRiotAccount"
                                @on-account-change="refreshRiotAccounts"
                                :options.sync="riotAccounts"
                            >
                            </riot-account-chooser>

                            <v-progress-circular size="64" indeterminate v-else></v-progress-circular>
                        </div>

                        <div class="mt-4">
                            You can always come back to this later by going to the game logs page for any supported Riot Games game. 
                            You can also see these instructions in more details in our <a href="#" @click="goToRiotAccountUserManual">user manual</a>.
                        </div>
                    </template>

                    <template v-else-if="st == DynamicSteps.Wow">
                        SquadOV depends on World of Warcraft's combat logs to determine when you are raiding, running keystones, or competing in the arena.
                        To ensure World of Warcraft is properly setup, ensure:

                        <ol class="mt-4">
                            <li>
                                You have an automatic combat logger addon installed.
                                We recommend using the <a href="#" @click="goToScl">Simple Combat Logger</a> addon.
                            </li>
                            <li>
                                In your World of Warcraft settings menu, enable the <b>Advanced Combat Logging</b> option under the "System" and "Network" submenus.
                            </li>
                        </ol>
                        
                        <div class="mt-4">
                            You can see these instructions in more details in our <a href="#" @click="goToWowUserManual">user manual</a>.
                        </div>
                    </template>

                    <template v-else-if="st == DynamicSteps.Csgo">
                        SquadOV works out of the box with CS:GO.
                        <b>However</b>, you should be aware that we do modify your autoexec file to achieve our automatic recording and squad sync.
                        In particular we add these lines to your autoexec.cfg:

                        <pre>
con_logfile squadov.log
con_timestamp 1
                        </pre>
                        If these settings already existed in your autoexec, they have been overwritten.
                        These settings are crucial for getting SquadOV to work with CS:GO.

                        Don't know what we're talking about? You can ignore everything we just said! Enjoy!
                    </template>

                    <template v-else-if="st == DynamicSteps.Squad">
                        SquadOV is all about squads which lets you easily sync VODs and view each other's recorded footage.
                        To get you started, we created you a default squad with Derek and Mike (co-founders of SquadOV) in them so your recent match feed isn't empty to start with.
                        Feel free to remove them whenever you want!

                        <div class="d-flex justify-center mt-4">
                            <squad-member-table
                                v-if="!!squadMembers"
                                v-model="squadMembers"
                                :is-owner="true"
                            >
                            </squad-member-table>

                            <v-progress-circular size="64" indeterminate v-else></v-progress-circular>
                        </div>

                        <div class="mt-4">
                            It's also super easy to invite your friend! Click the button below to invite your friend to your squad by username or by email!
                            If they're not already registered, they'll receive an email to sign up and join your squad! Simple!
                        </div>

                        <div class="d-flex justify-center mt-4">
                            <v-dialog persistent max-width="60%" v-model="showHideInviteSquad">
                                <template v-slot:activator="{on, attrs}">
                                    <v-btn
                                        v-on="on"
                                        v-bind="attrs"
                                        large
                                        small
                                        color="primary"
                                    >
                                        Invite
                                    </v-btn>
                                </template>

                                <squad-invite-create-card
                                    :squad-id="defaultSquad.squad.id"
                                    @on-cancel-invite="showHideInviteSquad = false"
                                    @on-send-invite="showHideInviteSquad = false"
                                >
                                </squad-invite-create-card>
                            </v-dialog>
                        </div>

                        <div class="mt-4">
                            You can read more about managing squads in our <a href="#" @click="goToSquadUserManual">user manual</a>.
                        </div>
                    </template>
                </v-stepper-content>

                <v-stepper-content :step="totalSteps">
                    <div class="d-flex flex-column justify-center align-center full-parent-height">
                        <div class="text-h6">
                            That's it! You're good to go, welcome to SquadOV.
                        </div>

                        <v-btn :to="dashboardTo" color="success" class="mt-4" large>
                            Exit Wizard
                        </v-btn>
                    </div>
                </v-stepper-content>
            </v-stepper-items>
        </v-stepper>

        <div class="d-flex justify-center" v-else>
            <v-progress-circular size="64" indeterminate></v-progress-circular>
        </div>

        <div class="d-flex align-center mt-2">
            <v-btn
                :to="dashboardTo"
                large
            >
                Exit Wizard
            </v-btn>

            <v-spacer></v-spacer>

            <v-btn
                class="mr-4"
                color="warning"
                @click="step -= 1"
                v-if="step > 1"
                large
            >
                Previous
            </v-btn>

            <v-btn
                color="primary"
                @click="step += 1"
                v-if="step != totalSteps"
                large
            >
                Next
            </v-btn>

            <v-btn
                color="success"
                :to="dashboardTo"
                large
                v-else
            >
                Finish
            </v-btn>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import * as pi from '@client/js/pages'
import { apiClient, ApiData } from '@client/js/api'
import { SquadMembership } from '@client/js/squadov/squad'
import { openUrlInBrowser } from '@client/js/external'
import { RiotAccountData } from '@client/js/valorant/valorant_account'
import RiotAccountChooser from '@client/vue/utility/riot/RiotAccountChooser.vue'
import SquadMemberTable from '@client/vue/utility/squads/SquadMemberTable.vue'
import SquadInviteCreateCard from '@client/vue/utility/squads/SquadInviteCreateCard.vue'

enum DynamicSteps {
    Riot,
    Wow,
    Squad,
    Csgo,
}

@Component({
    components: {
        RiotAccountChooser,
        SquadMemberTable,
        SquadInviteCreateCard
    }
})
export default class SetupWizard extends Vue {
    DynamicSteps: any = DynamicSteps

    step: number = 1
    dynamicSteps: Set<DynamicSteps> = new Set()
    defaultSquad: SquadMembership | null = null
    squadMembers: SquadMembership[] | null = null
    showHideInviteSquad: boolean = false
    ready: boolean = false

    gameSteps: DynamicSteps[] = []

    // Riot Account Data
    selectedRiotAccount: RiotAccountData | null = null
    riotAccounts: RiotAccountData[] | null = null

    stepToString(s: DynamicSteps): string {
        switch(s) {
            case DynamicSteps.Riot:
                return 'Riot Account'
            case DynamicSteps.Wow:
                return 'World of Warcraft'
            case DynamicSteps.Squad:
                return 'Squads'
            case DynamicSteps.Csgo:
                return 'CS:GO'
        }
    }

    get totalSteps(): number {
        return 2 + this.dynamicSteps.size
    }

    get dashboardTo(): any {
        return {
            name: pi.DashboardPageId
        }
    }

    @Watch('gameSteps')
    onChangeGameSteps() {
        let gs = new Set(this.gameSteps)
        if (!!this.defaultSquad) {
            gs.add(DynamicSteps.Squad)
        }

        this.dynamicSteps = gs
    }

    goToWowUserManual() {
        openUrlInBrowser('https://support.squadov.gg/user-manual/games/wow#combatlog')
    }

    goToRiotAccountUserManual() {
        openUrlInBrowser('https://support.squadov.gg/user-manual/games/valorant#account')
    }

    goToScl() {
        const url = 'https://www.curseforge.com/wow/addons/simplecombatlogger'
        openUrlInBrowser(url)
    }

    goToSquadUserManual() {
        openUrlInBrowser('https://support.squadov.gg/user-manual/squads')
    }

    refreshRiotAccounts() {
        this.riotAccounts = null
        apiClient.listRiotValorantAccounts(this.$store.state.currentUser.id).then((resp : ApiData<RiotAccountData[]>) => {
            this.riotAccounts = resp.data
        }).catch((err : any) => {
            console.log('Failed to list valorant accounts: ', err);
        })
    }

    refreshSquadMembers() {
        if (!this.defaultSquad) {
            this.squadMembers = []
            return
        }

        this.squadMembers = null
        apiClient.getSquadUsers(this.defaultSquad.squad.id).then((resp: ApiData<SquadMembership[]>) => {
            this.squadMembers = resp.data
        }).catch((err: any) => {
            console.log('Failed to get users in default squad in setup wizard: ', err)
        })
    }

    mounted() {
        this.$store.commit('setSetupWizardRun', true)
        // Get a list of the user squads - find the default squad if it exists
        // and present it to the users during the setup wizard as the squad to mange.
        apiClient.getUserSquads(this.$store.state.currentUser.id).then((resp: ApiData<SquadMembership[]>) => {
            let username = this.$store.state.currentUser.username
            for (let sm of resp.data) {
                if (sm.squad.squadGroup === username && sm.squad.squadName === `${username}'s Squad`) {
                    this.defaultSquad = sm
                    this.dynamicSteps = new Set([DynamicSteps.Squad])
                    this.refreshSquadMembers()
                    break
                }
            }
        }).catch((err: any) => {
            console.log('Failed to get user squads in setup wizard: ', err)
        }).finally(() => {
            this.ready = true
        })

        this.refreshRiotAccounts()
    }
}

</script>

<style scoped>

.game {
    transition: opacity 0.5s linear 0s;
    cursor: pointer;
}

.chosen-game {
    opacity: 1.0;
}

.not-chosen-game {
    opacity: 0.3;
}

</style>