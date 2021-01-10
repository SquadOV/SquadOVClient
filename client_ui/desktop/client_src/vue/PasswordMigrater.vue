<template>
    <loading-container :is-loading="!accounts || accounts.length === 0 || accountIndex >= accounts.length">
        <template v-slot:default="{ loading }">
            <v-container fluid class="full-parent-height">
                <v-row class="full-parent-height" justify="center" align="center">
                    <v-col cols="8" align="center">
                        <div class="d-flex flex-column justify-center ma-4" v-if="!loading">
                            <div class="text-h5">
                                Please re-enter your VALORANT account credentials. ({{ accountIndex + 1 }} / {{ accounts.length }})
                            </div>
                            <v-divider class="my-4"></v-divider>
                            <v-form v-if="!!currentAccount">
                                <v-text-field
                                    class="my-3"
                                    :value="currentAccount.login"
                                    label="Username"
                                    outlined
                                    hide-details
                                    readonly
                                >
                                </v-text-field>

                                <v-text-field
                                    class="my-3"
                                    v-model="workingPassword"
                                    label="Password"
                                    outlined
                                    type="password"
                                    hide-details
                                >
                                </v-text-field>
                            </v-form>
                            <v-divider class="my-4"></v-divider>
                            <div class="d-flex" v-if="!!currentAccount">
                                <v-spacer></v-spacer>
                                <v-btn
                                    @click="skip"
                                    class="mr-4"
                                >
                                    Skip
                                </v-btn>

                                <v-btn
                                    color="success"
                                    @click="save"
                                >
                                    Save
                                </v-btn>
                            </div>
                        </div>
                    </v-col>
                </v-row>
            </v-container>

            <v-snackbar
                v-model="showHideError"
                :timeout="5000"
                color="error"
            >
                Failed to edit your VALORANT account credentials. Please re-enter your password and try again.
            </v-snackbar>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { RiotAccountData } from '@client/js/valorant/valorant_account'
import { apiClient, ApiData } from '@client/js/api'
import { ipcRenderer } from 'electron'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'

@Component({
    components: {
        LoadingContainer,
    }
})
export default class PasswordMigrater extends Vue {
    accounts : RiotAccountData[] | null = null
    accountIndex: number = 0
    workingPassword: string = ''
    saveInProgress: boolean = false
    showHideError: boolean = false

    get currentAccount() : RiotAccountData | null {
        if (!this.accounts || this.accountIndex >= this.accounts.length) {
            return null
        }
        return this.accounts[this.accountIndex]
    }

    skip() {
        if (!this.accounts) {
            return
        }

        this.workingPassword = ''
        this.accountIndex += 1
        if (this.accountIndex >= this.accounts.length) {
            this.onFinish()
        }
    }

    save() {
        if (!this.accounts || !this.currentAccount) {
            return
        }

        this.saveInProgress = true
        apiClient.editValorantAccount(this.currentAccount.puuid, this.currentAccount.login!, this.workingPassword).then(() => {
            console.log('Successfully migrated VALORANT account: ', this.currentAccount!.login!)
            this.skip()
        }).catch((err: any) => {
            console.log('Failed to migrate VALORANT account: ', err)
            this.showHideError = true
        }).finally(() => {
            this.saveInProgress = false
        })
    }

    onFinish() {
        ipcRenderer.send('finish')
    }

    refreshValorantAccounts() {
        apiClient.listLocalValorantAccounts().then((resp: ApiData<RiotAccountData[]>) => {
            this.accounts = resp.data.filter((ele: RiotAccountData) => {
                return ele.isMigrated === 0
            })
            console.log('Listed VALORANT accounts: ', resp.data.length, this.accounts!.length)

            if (this.accounts!.length == 0) {
                this.onFinish()
            }
        }).catch((err: any) => {
            console.log('Failed to list local valorant accounts: ', err)
            this.onFinish()
        })
    }

    mounted() {
        this.refreshValorantAccounts()
    }

}

</script>