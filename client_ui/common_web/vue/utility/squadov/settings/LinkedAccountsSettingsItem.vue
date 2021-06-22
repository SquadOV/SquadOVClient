<template>
    <div class="full-width">
        <v-dialog
            v-model="showHideDelete"
            persistent
            max-width="60%"
        >
            <v-card>
                <v-card-title>
                    Are you sure you wish to unlink this account?
                </v-card-title>
                <v-divider></v-divider>

                <v-card-text class="mt-4">
                    <template v-if="!!riotToDelete">
                        Are you sure you wish to unlink your Riot account ({{ riotToDelete.gameName }}#{{ riotToDelete.tagLine }})?
                        You will no longer be able to see any of your matches and VODs tied to this account.
                    </template>

                    <template v-if="!!twitchToDelete">
                        Are you sure you wish to unlink your Twitch account ({{ twitchToDelete.twitchName }})?
                        You will no longer be a part of or be able to join communities that require a Twitch subscription to the creator. 
                    </template>
                </v-card-text>

                <v-card-actions>
                    <v-btn
                        @click="cancelDelete"
                    >
                        Cancel
                    </v-btn>

                    <v-spacer></v-spacer>

                    <v-btn
                        color="error"
                        :loading="deletePending"
                        @click="unlinkAccount"
                    >
                        Unlink
                    </v-btn>
                </v-card-actions>
            </v-card>
        </v-dialog>

        <div class="d-flex align-center my-2">
            <div class="font-weight-bold text-h6">
                Linked Accounts
            </div>

            <v-menu offset-y>
                <template v-slot:activator="{on, attrs}">
                    <v-btn
                        v-on="on"
                        v-bind="attrs"
                        class="ml-2"
                        text
                        color="primary"
                    >
                        <v-icon>
                            mdi-plus
                        </v-icon>

                        <v-icon x-small>
                            mdi-chevron-down
                        </v-icon>
                    </v-btn>
                </template>

                <v-list dense>
                    <v-list-item @click="linkRiot">
                        <v-list-item-avatar tile>
                            <v-img width="32px" max-width="32px" contain :src="$root.generateAssetUri('assets/logos/riot.png')"></v-img>
                        </v-list-item-avatar>

                        <v-list-item-content>
                            <v-list-item-title>
                                Riot
                            </v-list-item-title>
                        </v-list-item-content>
                    </v-list-item>

                    <v-list-item @click="linkTwitch">
                        <v-list-item-avatar tile>
                            <v-img width="32px" max-width="32px" contain :src="$root.generateAssetUri('assets/logos/twitch.png')"></v-img>
                        </v-list-item-avatar>

                        <v-list-item-content>
                            <v-list-item-title>
                                Twitch
                            </v-list-item-title>
                        </v-list-item-content>
                    </v-list-item>
                </v-list>
            </v-menu>

            <v-btn
                class="ml-2"
                icon
                @click="refreshData"
            >
                <v-icon>
                    mdi-refresh
                </v-icon>
            </v-btn>
        </div>
        <v-divider></v-divider>

        <loading-container :is-loading="!accounts">
            <template v-slot:default="{ loading }">
                <div v-if="!loading">
                    <div class="text-overline font-weight-bold">
                        Riot Accounts
                    </div>

                    <div class="d-flex flex-wrap">
                        <div
                            class="account-div d-flex align-center"
                            v-for="(account, idx) in accounts.riot"
                            :key="`riot-${idx}`"
                        >
                            <v-img width="64px" max-width="64px" contain :src="$root.generateAssetUri('assets/logos/riot.png')"></v-img>

                            <div class="text-6 font-weight-bold ml-4">
                                {{ account.gameName }}#{{ account.tagLine }}
                            </div>

                            <v-spacer></v-spacer>

                            <v-btn color="error" icon small class="ml-4" @click="deleteRiot(account)">
                                <v-icon>
                                    mdi-close-circle
                                </v-icon>
                            </v-btn>
                        </div>
                    </div>

                    <div class="text-overline font-weight-bold mt-4">
                        Twitch Accounts
                    </div>

                    <div class="d-flex flex-wrap">
                        <div
                            class="account-div d-flex align-center"
                            v-for="(account, idx) in accounts.twitch"
                            :key="`twitch-${idx}`"
                        >
                            <v-img width="64px" max-width="64px" contain :src="$root.generateAssetUri('assets/logos/twitch.png')"></v-img>

                            <div class="text-h6 font-weight-bold ml-4">
                                {{ account.twitchName }}
                            </div>

                            <v-spacer></v-spacer>

                            <v-btn color="error" icon small class="ml-4" @click="deleteTwitch(account)">
                                <v-icon>
                                    mdi-close-circle
                                </v-icon>
                            </v-btn>
                        </div>
                    </div>
                </div>
            </template>
        </loading-container>

        <v-snackbar
            v-model="deleteFailure"
            :timeout="5000"
            color="error"
        >
            Failed to unlink your account, please try again later.
        </v-snackbar>

        <v-snackbar
            v-model="showHideProgress"
            :timeout="5000"
            color="success"
        >
            Please login using in the service provider's login form that was just opened in your web browser.
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { LinkedAccounts, TwitchAccount } from '@client/js/squadov/accounts'
import { apiClient, ApiData } from '@client/js/api'
import { RiotAccountData } from '@client/js/valorant/valorant_account'
import { redirectToRsoLogin } from '@client/js/riot/rso'
import { redirectToTwitchLogin } from '@client/js/twitch'

@Component({
    components: {
        LoadingContainer,
    }
})
export default class LinkedAccountsSettingsItem extends Vue {
    accounts: LinkedAccounts | null = null

    showHideDelete: boolean = false
    deletePending: boolean = false
    deleteFailure: boolean = false

    riotToDelete: RiotAccountData | null = null
    twitchToDelete: TwitchAccount | null = null

    pendingOauthLink: boolean = false
    showHideProgress: boolean = false

    deleteRiot(account: RiotAccountData) {
        this.riotToDelete = account
        this.showHideDelete = true
    }

    deleteTwitch(account: TwitchAccount) {
        this.twitchToDelete = account
        this.showHideDelete = true
    }

    cancelDelete() {
        this.riotToDelete = null
        this.twitchToDelete = null
        this.showHideDelete = false
    }

    unlinkAccount() {
        let promise
        if (!!this.riotToDelete) {
            promise = apiClient.deleteRiotAccount(this.$store.state.currentUser.id, this.riotToDelete.puuid)
        } else if (!!this.twitchToDelete) {
            promise = apiClient.deleteMyLinkedTwitchAccount(this.twitchToDelete.twitchUserId)
        } else {
            this.cancelDelete()
            return
        }

        this.deletePending = true
        promise.then(() => {
            if (!!this.accounts) {
                let idx = !!this.twitchToDelete ? this.accounts.twitch.findIndex((ele: TwitchAccount) => ele.twitchUserId === this.twitchToDelete!.twitchUserId) :
                    !!this.riotToDelete ? this.accounts.riot.findIndex((ele: RiotAccountData) => ele.puuid === this.riotToDelete!.puuid) :
                    -1

                if (idx != -1) {
                    if (!!this.twitchToDelete) {
                        this.accounts.twitch.splice(idx, 1)
                    } else if (!!this.riotToDelete) {
                        this.accounts.riot.splice(idx, 1)
                    }
                }
            }

            this.twitchToDelete = null
            this.riotToDelete = null
            this.showHideDelete = false
        }).catch((err: any) => {
            console.log('Failed to unlink account: ', err)
            this.deleteFailure = true
        }).finally(() => {
            this.deletePending = false
        })
    }

    @Watch('$store.state.currentUser')
    refreshData() {
        this.accounts = null
        apiClient.getAllLinkedAccounts().then((resp: ApiData<LinkedAccounts>) => {
            this.accounts = resp.data
        }).catch((err: any) => {
            console.log('Failed to get linked accounts: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }

    linkRiot() {
        this.pendingOauthLink = true
        redirectToRsoLogin(this.$store.state.currentUser.id).then(() => {
            this.showHideProgress = true
        }).catch((err: any) => {
            console.log('Failed to redirect to RSO: ', err)
        }).finally(() => {
            this.pendingOauthLink = false
        })
    }

    linkTwitch() {
        this.pendingOauthLink = true
        redirectToTwitchLogin().then(() => {
            this.showHideProgress = true
        }).catch((err: any) => {
            console.log('Failed to redirect to Twitch: ', err)
        }).finally(() => {
            this.pendingOauthLink = false
        })
    }
}

</script>

<style scoped>

.account-div {
    width: 300px;
    min-height: 100px;
    padding: 16px;
    border: 1px solid #6E6E6E;
    margin-right: 16px;
}

</style>