<template>
    <div class="d-flex flex-column full-parent-height pa-4 outer-content">
        <div class="inner-content">
            <v-stepper v-model="step" class="flex-grow-1" non-linear v-if="ready">
                <v-stepper-header>
                    <v-stepper-step
                        :complete="step > 1"
                        :step="1"
                    >
                        Welcome
                    </v-stepper-step>

                    <v-stepper-step
                        v-for="(st, idx) in dynamicSteps"
                        :key="`header-${idx}`"
                        :complete="step > (idx + 2)"
                        :step="idx + 2"
                    >
                        {{ stepToString(st) }}
                    </v-stepper-step>

                    <v-stepper-step
                        :complete="false"
                        :step="totalSteps"
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

                            <v-item-group class="d-flex flex-wrap justify-center align-center" multiple v-model="gameSteps">
                                <v-item v-slot="{active, toggle}" :value="DynamicSteps.Csgo">
                                    <v-img
                                        :class="`game ${active ? 'chosen-game' : 'not-chosen-game'} mx-2`"
                                        :width="imageWidth"
                                        :max-width="imageWidth"
                                        :src="$root.generateAssetUri('assets/csgo_box.jpg')"
                                        contain
                                        @click="toggle"
                                    >
                                    </v-img>
                                </v-item>

                                <v-item v-slot="{active, toggle}" :value="DynamicSteps.Wow">
                                    <v-img
                                        :class="`game ${active ? 'chosen-game' : 'not-chosen-game'} mx-2`"
                                        :width="imageWidth"
                                        :max-width="imageWidth"
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

                            <v-checkbox
                                class="mt-4"
                                v-model="doneSelect"
                                label="I've selected the games I want SquadOV to record going forward (and I know that games I played in the past won't have VODs)!"
                                color="success"
                                hide-details
                            >
                            </v-checkbox>

                            <setup-wizard-stepper
                                class="mt-4"
                                v-model="step"
                                :total-steps="totalSteps"
                                :can-go-next="canGoNext"
                            >
                            </setup-wizard-stepper>
                        </div>
                    </v-stepper-content>

                    <v-stepper-content
                        v-for="(st, idx) in dynamicSteps"
                        :key="`content-${idx}`"
                        :step="idx + 2"
                    >
                        <template v-if="st == DynamicSteps.Wow">
                            SquadOV depends on World of Warcraft's combat logs to determine when you are raiding, running keystones, or competing in the arena.
                            To ensure World of Warcraft is properly setup, ensure:

                            <ol class="mt-4">
                                <li>
                                    You have an automatic combat logger addon installed.
                                    We recommend using the <a href="#" @click="goToScl">Simple Combat Logger</a> addon if you're playing retail.
                                    We recommend using the <a href="#" @click="goToAcl">AutoCombatLogger</a> addon if you're playing classic (TBC or Vanilla).
                                </li>
                                <li>
                                    In your World of Warcraft settings menu, enable the <b>Advanced Combat Logging</b> option under the "System" and "Network" submenus.
                                </li>
                            </ol>

                            <div class="mt-4">
                                <b>You must also disable any other addons (or their features) that may enable the combat log (Method Raid Tools, LoggerHead, Arena Combat Logger, etc.).</b>
                            </div>

                            <div class="mt-4 d-flex justify-center">
                                <v-checkbox
                                    v-model="doneWow"
                                    label="I've installed the Simple Combat Logger addon and enabled Advanced Combat Logging. I'm all good to go!"
                                    color="success"
                                    hide-details
                                >
                                </v-checkbox>
                            </div>

                            <setup-wizard-stepper
                                class="mt-4"
                                v-model="step"
                                :total-steps="totalSteps"
                                :can-go-next="canGoNext"
                            >
                            </setup-wizard-stepper>
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
                            <b>Note that we do not currently support FaceIT or ESEA.</b>

                            <div class="mt-4 d-flex justify-center">
                                <v-checkbox
                                    v-model="doneCsgo"
                                    label="Yup, read all that and I know that my autoexec is going to change!"
                                    color="success"
                                    hide-details
                                >
                                </v-checkbox>
                            </div>

                            <setup-wizard-stepper
                                class="mt-4"
                                v-model="step"
                                :total-steps="totalSteps"
                                :can-go-next="canGoNext"
                            >
                            </setup-wizard-stepper>
                        </template>

                        <template v-else-if="st == DynamicSteps.Squad">
                            SquadOV is all about squads which lets you easily sync VODs and view each other's recorded footage.
                            SquadOV is better with friends!

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
                                It's also super easy to invite your friend! Click the button below to invite your friend to your squad.
                                You can either send them a link or invite them directly by username or email. Easy!
                            </div>

                            <div class="d-flex justify-center mt-4">
                                <v-dialog max-width="40%" v-model="showHideInviteSquad">
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
                                        @on-send-invite="showHideInviteSquad = false"
                                    >
                                    </squad-invite-create-card>
                                </v-dialog>
                            </div>
                            
                            <div class="mt-4 d-flex justify-center">
                                <v-checkbox
                                    v-model="doneSquad"
                                    label="I've invited my friends to SquadOV because this is going to be awesome!"
                                    color="success"
                                    hide-details
                                >
                                </v-checkbox>
                            </div>

                            <setup-wizard-stepper
                                class="mt-4"
                                v-model="step"
                                :total-steps="totalSteps"
                                :can-go-next="canGoNext"
                            >
                            </setup-wizard-stepper>
                        </template>
                    </v-stepper-content>

                    <v-stepper-content :step="totalSteps">
                        <div class="d-flex flex-column justify-center align-center full-parent-height">
                            <template v-if="!isDesktop">
                                <div class="text-h6">
                                    One last step! Download SquadOV and login to our desktop app to start recording your VODs.
                                </div>

                                <download-button class="mt-4" large setup-wizard></download-button>
                            </template>

                            <div class="mt-4">
                                Also, join our Discord! We're super active there and bug reports and feature requests there generally get answered quicker.
                            </div>

                            <v-btn
                                class="mt-4"
                                :href="discordUrl"
                                target="_blank"
                                @click="onGoToDiscord"
                                color="primary"
                                v-if="!isDesktop"
                            >
                                Discord
                            </v-btn>

                            <v-btn
                                class="mt-4"
                                @click="goToLink(discordUrl)"
                                color="primary"
                                v-else
                            >
                                Discord
                            </v-btn>

                            <v-checkbox
                                class="mt-4"
                                v-model="doneDownload"
                                label="I've downloaded SquadOV because I can't record without downloading and installing it! I've also joined Discord because I'm awesome."
                                color="success"
                                hide-details
                            >
                            </v-checkbox>

                            <setup-wizard-stepper
                                class="mt-4"
                                v-model="step"
                                :total-steps="totalSteps"
                                :can-go-next="canGoNext"
                            >
                            </setup-wizard-stepper>
                        </div>
                    </v-stepper-content>
                </v-stepper-items>
            </v-stepper>

            <div class="d-flex justify-center" v-else>
                <v-progress-circular size="64" indeterminate></v-progress-circular>
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import CommonComponent from '@client/vue/CommonComponent'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { SquadMembership } from '@client/js/squadov/squad'
import { openUrlInBrowser } from '@client/js/external'
import { RiotAccountData } from '@client/js/valorant/valorant_account'
import RiotAccountChooser from '@client/vue/utility/riot/RiotAccountChooser.vue'
import SquadMemberTable from '@client/vue/utility/squads/SquadMemberTable.vue'
import SquadInviteCreateCard from '@client/vue/utility/squads/SquadInviteCreateCard.vue'
import DownloadButton from '@client/vue/utility/DownloadButton.vue'
import SetupWizardStepper from '@client/vue/utility/SetupWizardStepper.vue'
import { AnalyticsAction, AnalyticsCategory } from '@client/js/analytics/events'

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
        SquadInviteCreateCard,
        DownloadButton,
        SetupWizardStepper,
    }
})
export default class SetupWizard extends CommonComponent {
    DynamicSteps: any = DynamicSteps

    step: number = 1
    dynamicSteps: Set<DynamicSteps> = new Set()
    defaultSquad: SquadMembership | null = null
    squadMembers: SquadMembership[] | null = null
    showHideInviteSquad: boolean = false
    ready: boolean = false

    gameSteps: DynamicSteps[] = []

    doneSelect: boolean = false
    doneRiot: boolean = false
    doneWow: boolean = false
    doneSquad: boolean = false
    doneCsgo: boolean = false
    doneDownload: boolean = false

    // Riot Account Data
    selectedRiotAccount: RiotAccountData | null = null
    riotAccounts: RiotAccountData[] | null = null

    get isDesktop(): boolean {
/// #if DESKTOP
        return true
/// #else
        return false
/// #endif
    }

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

    get canGoNext(): boolean {
        if (this.step == 1) {
            return this.doneSelect
        } else if (this.step == this.totalSteps) {
            return this.doneDownload
        } else {
            let idx = this.step - 2
            if (idx < this.gameSteps.length) {
                let realStep = this.gameSteps[idx]
                switch(realStep) {
                    case DynamicSteps.Riot:
                        return this.doneRiot
                    case DynamicSteps.Wow:
                        return this.doneWow
                    case DynamicSteps.Csgo:
                        return this.doneCsgo
                }
            } else {
                return this.doneSquad
            }
        }

        return false
    }

    get imageWidth(): number {
        return 128;
    }

    get totalSteps(): number {
        return 2 + this.dynamicSteps.size
    }

    @Watch('gameSteps')
    onChangeGameSteps() {
        let gs = new Set(this.gameSteps)
        if (!!this.defaultSquad) {
            gs.add(DynamicSteps.Squad)
        }

        this.dynamicSteps = gs
    }

    @Watch('step')
    onChangeSteps() {
        if (this.step == 1) {
            this.analytics?.event(this.$route, AnalyticsCategory.SetupWizard, AnalyticsAction.SetupStartSelectGames, '', 0)
        } else if (this.step === this.totalSteps) {
            this.analytics?.event(this.$route, AnalyticsCategory.SetupWizard, AnalyticsAction.SetupStartDownloadDiscord, '', 0)
///#if !DESKTOP
            localStorage.setItem('squadovSetup', 'yes')
///#endif
        } else {
            let idx = this.step - 2
            if (idx < this.gameSteps.length) {
                let realStep = this.gameSteps[idx]
                switch(realStep) {
                    case DynamicSteps.Riot:
                        this.analytics?.event(this.$route, AnalyticsCategory.SetupWizard, AnalyticsAction.SetupStartRiot, '', 0)
                    case DynamicSteps.Wow:
                        this.analytics?.event(this.$route, AnalyticsCategory.SetupWizard, AnalyticsAction.SetupStartWow, '', 0)
                    case DynamicSteps.Csgo:
                        this.analytics?.event(this.$route, AnalyticsCategory.SetupWizard, AnalyticsAction.SetupStartCsgo, '', 0)
                }
            } else {
                this.analytics?.event(this.$route, AnalyticsCategory.SetupWizard, AnalyticsAction.SetupStartSquads, '', 0)
            }
        }
    }

    get discordUrl(): string {
        return 'https://discord.gg/6Rj5jCVDeC'
    }

    goToScl() {
        const url = 'https://www.curseforge.com/wow/addons/simplecombatlogger'
        openUrlInBrowser(url)
    }

    goToAcl() {
        const url = 'https://www.curseforge.com/wow/addons/autocombatlogger'
        openUrlInBrowser(url)
    }

    refreshRiotAccounts() {
        this.riotAccounts = null
        apiClient.listRiotValorantAccounts(this.$store.state.currentUser.id).then((resp : ApiData<RiotAccountData[]>) => {
            this.riotAccounts = resp.data
        }).catch((err : any) => {
            console.error('Failed to list valorant accounts: ', err);
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
            console.error('Failed to get users in default squad in setup wizard: ', err)
        })
    }

    startSetupWizard() {
        console.log('Starting Setup Wizard...')
        this.$store.commit('setSetupWizardRun', true)
        // Get a list of the user squads - find the default squad if it exists
        // and present it to the users during the setup wizard as the squad to mange.
        apiClient.getUserSquads(this.$store.state.currentUser.id).then((resp: ApiData<SquadMembership[]>) => {
            let username = this.$store.state.currentUser.username
            for (let sm of resp.data) {
                if (sm.squad.squadName === `${username}'s Squad`) {
                    this.defaultSquad = sm
                    this.dynamicSteps = new Set([DynamicSteps.Squad])
                    this.refreshSquadMembers()
                    break
                }
            }
        }).catch((err: any) => {
            console.error('Failed to get user squads in setup wizard: ', err)
        }).finally(() => {
            this.ready = true
        })

        this.refreshRiotAccounts()

        this.analytics?.event(this.$route, AnalyticsCategory.SetupWizard, AnalyticsAction.StartSetupWizard, '', 0)
    }

    @Watch('isActive')
    onActiveChange() {
        if (this.isActive) {
            this.startSetupWizard()
        }
    }

    mounted() {
        this.startSetupWizard()
    }

    goToLink(url: string) {
        openUrlInBrowser(url)
        if (url == this.discordUrl) {
            this.onGoToDiscord()
        }
    }

    onGoToDiscord() {
        this.analytics?.event(this.$route, AnalyticsCategory.SetupWizard, AnalyticsAction.SetupJoinDiscord, '', 0)
    }
}

</script>

<style scoped>

.game {
    transition: opacity 0.5s linear 0s;
    cursor: pointer;
    margin-top: 8px;
    margin-bottom: 8px;
}

.chosen-game {
    opacity: 1.0;
}

.not-chosen-game {
    opacity: 0.3;
}

.outer-content {
    overflow: auto;
}

.inner-content {
    width: 60%;
    max-width: 60%;
    margin: auto;
}

</style>