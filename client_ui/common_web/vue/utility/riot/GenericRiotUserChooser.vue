<template>
    <div class="d-flex align-center">
        <v-dialog v-model="showHideDeleteConfirm" persistent max-width="40%">
            <template v-slot:activator="{on, attrs}">
                <v-btn
                    class="mr-2"
                    icon
                    color="error"
                    @click="showHideDeleteConfirm = true"
                    :disabled="!puuid"
                    :loading="loadingDelete"
                    v-on="on"
                    v-bind="attrs"
                >
                    <v-icon>
                        mdi-delete
                    </v-icon>
                </v-btn>
            </template>

            <v-card>
                <v-card-title>
                    Confirmation
                </v-card-title>
                <v-divider></v-divider>

                <v-card-text class="mt-4">
                    <div>
                        Are you sure you wish to delete {{ account }}? You will no longer record VODs automatically when playing on this account.
                    </div>

                    <div class="mt-4">
                        Please type <span class="font-weight-bold">CONFIRM</span> (case-sensitive) to confirm this action.
                    </div>

                    <v-text-field
                        class="mt-4"
                        filled
                        label="Confirmation"
                        v-model="confirmationText"
                        hide-details
                    >
                    </v-text-field>
                </v-card-text>

                <v-card-actions>
                    <v-btn @click="cancelDelete">
                        Cancel
                    </v-btn>

                    <v-spacer></v-spacer>

                    <v-btn
                        color="error"
                        :loading="loadingDelete"
                        :disabled="confirmationText != 'CONFIRM'"
                        @click="deleteAccount"
                    >
                        Delete
                    </v-btn>
                </v-card-actions>
            </v-card>
        </v-dialog>

        <slot></slot>

        <v-btn
            class="mr-2"
            icon
            color="success"
            @click="addAccount"
            :loading="loadingAdd"
        >
            <v-icon>
                mdi-plus
            </v-icon>
        </v-btn>

        <v-btn
            icon
            color="primary"
            @click="resyncAccount"
            :disabled="!puuid"
            :loading="loadingSync"
        >
            <v-icon>
                mdi-swap-vertical
            </v-icon>
        </v-btn>

        <v-snackbar
            v-model="showHideProgress"
            :timeout="5000"
            color="success"
        >
            Please login using Riot's login form that was just opened in your web browser.
        </v-snackbar>

        <v-snackbar
            v-model="showHideResync"
            :timeout="5000"
            color="success"
        >
            We are currently resyncing your account, this may take a few moments.
        </v-snackbar>

        <v-snackbar
            v-model="showError"
            :timeout="5000"
            color="error"
        >
            Something went wrong when processing your request. Please submit a bug report!
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { redirectToRsoLogin } from '@client/js/riot/rso'
import { apiClient } from '@client/js/api'

@Component
export default class GenericRiotUserChooser extends Vue {
    @Prop({required: true})
    puuid!: string | null

    @Prop({required: true})
    account!: string | null

    loadingAdd: boolean = false
    loadingDelete: boolean = false
    loadingSync: boolean = false

    showHideDeleteConfirm: boolean = false
    confirmationText: string = ''

    showError: boolean = false
    showHideProgress: boolean = false
    showHideResync: boolean = false

    cancelDelete() {
        this.confirmationText = ''
        this.showHideDeleteConfirm = false
    }

    addAccount() {
        this.loadingAdd = true
        redirectToRsoLogin(this.$store.state.currentUser.id).then((url: string) => {
            this.$store.commit('setRedirectUrl', url)
            this.showHideProgress = true
        }).catch((err: any) => {
            console.error('Failed to redirect to RSO: ', err)
            this.showError = true
        }).finally(() => {
            this.loadingAdd = false
        })
    }

    deleteAccount() {
        if (!this.puuid) {
            return
        }

        this.loadingDelete = true
        apiClient.deleteRiotAccount(this.$store.state.currentUser.id, this.puuid).then(() => {
            this.$emit('on-account-change')
            this.showHideDeleteConfirm = false
        }).catch((err: any) => {
            console.error('Failed to delete riot account: ', err)
            this.showError = true
        }).finally(() => {
            this.loadingDelete = false
        })
    }

    resyncAccount() {
        if (!this.puuid) {
            return
        }

        this.loadingSync = true
        apiClient.resyncRiotAccount(this.$store.state.currentUser.id, this.puuid).then(() => {
            this.showHideResync = true
        }).catch((err: any) => {
            console.error('Failed to resync riot account: ', err)
            this.showError = true
        }).finally(() => {
            this.loadingSync = false
        })
    }
}

</script>