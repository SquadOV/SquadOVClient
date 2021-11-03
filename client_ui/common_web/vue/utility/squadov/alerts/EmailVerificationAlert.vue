<template>
    <div v-if="isLoggedIn">
        <v-alert
            v-if="showEmailVerification"
            class="pa-2 ma-0"
            type="warning"
            dismissible
            tile
            width="100%"
            @input="onClose"
        >
            <template v-slot:prepend>
                <v-icon>mdi-exclamation</v-icon>
            </template>

            <div class="d-flex align-center" height="emailAlertHeight">
                <div>
                    Your email address is unverified. Please verify your email
                    address ({{ emailAddress }}) ASAP to prevent account
                    deletion.
                </div>

                <v-spacer></v-spacer>

                <v-btn
                    small
                    color="primary"
                    @click="resendVerification"
                    v-if="!sentVerification"
                    :loading="emailPending"
                >
                    Resend Email Verification
                </v-btn>

                <div v-else>
                    Email Sent!
                </div>

                <v-btn
                    small
                    v-if="!sentVerification"
                    class="ml-2"
                    color="secondary"
                    :to="accountSettingsTo"
                >
                    Wrong email!
                </v-btn>

                <v-btn
                    class="ml-2"
                    color="success"
                    small
                    icon
                    @click="recheckVerification"
                    :loading="checkPending"
                >
                    <v-icon>
                        mdi-refresh
                    </v-icon>
                </v-btn>
            </div>
        </v-alert>
        <v-snackbar v-model="verificationErr" :timeout="5000" color="error">
            Failed to send a verification email. Please try again later.
        </v-snackbar>
    </div>
</template>

<script lang="ts">
import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { getSquadOVUser, SquadOVUser } from '@client/js/squadov/user'
import * as pi from '@client/js/pages'

@Component
export default class EmailVerificationAlert extends Vue {
    checkPending: boolean = false
    showEmailVerification: boolean = false
    emailPending: boolean = false
    sentVerification: boolean = false
    verificationErr: boolean = false
    emailAlertHeight: number = 45

    get accountSettingsTo(): any {
        return {
            name: pi.SettingsPageId,
            query: {
                inputTab: 2,
            },
        }
    }

    recheckVerification() {
        this.checkPending = true
        getSquadOVUser(this.$store.state.currentUser.id)
            .then((resp: ApiData<SquadOVUser>) => {
                this.$store.commit('setUser', resp.data)
            })
            .catch((err: any) => {
                console.error('Failed to refresh squadovuser : ', err)
            })
            .finally(() => {
                this.checkPending = false
            })
    }

    get isLoggedIn(): boolean {
        return !!this.$store.state.currentUser
    }

    get emailAddress(): string {
        if (!this.$store.state.currentUser) {
            return ''
        }
        return this.$store.state.currentUser.email
    }

    onClose() {
        this.muteAlert()
    }

    muteAlert() {
        window.sessionStorage.setItem('muteEmailAlert', 'true')
    }

    get isAlarmMute() {
        return (window.sessionStorage.getItem('muteEmailAlert') == 'true' ? true : false)
    }

    resendVerification() {
        this.emailPending = true
        apiClient
            .resendVerification()
            .then(() => {
                this.sentVerification = true
                setTimeout(() => {
                    this.showEmailVerification = false
                }, 5000)
            })
            .catch((err: any) => {
                console.error('Failed to resend verification: ', err)
                this.verificationErr = false
            })
            .finally(() => {
                this.emailPending = false
            })
    }

    mounted() {
        this.resyncVerificationStatus()
    }

    @Watch('$store.state.currentUser')
    resyncVerificationStatus() {
        if (this.isLoggedIn && !this.isAlarmMute) {
            this.showEmailVerification = !this.$store.state.currentUser.verified
        }
    }
}
</script>
