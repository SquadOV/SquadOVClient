<template>
    <v-dialog
        v-if="!!$store.state.currentUser"
        v-model="showPopup"
        persistent
        max-width="40%"
    >
        <v-card>
            <v-card-title>
                Verify your Email Address!
            </v-card-title>
            <v-divider class="my-2"></v-divider>

            <v-card-text>
                <span class="font-weight-bold">We will begin disabling access for non-verified users soon^TM.</span> Help us protect your account and verify your email address.
            </v-card-text>

            <template v-if="!editEmail">
                <v-card-text>
                    The email address we have on file for you is:
                </v-card-text>

                <v-card-text class="d-flex justify-center text-h6 font-weight-bold">
                    {{ $store.state.currentUser.email }}
                </v-card-text>

                <v-card-text v-if="stillNoVerify" class="d-flex justify-center">
                    Still not verified yet, it could take a bit (but not too many bits). :(
                </v-card-text>

                <div class="d-flex justify-center align-center pb-4">
                    <div class="d-flex flex-column justify-center align-center button-actions">
                        <v-btn
                            v-if="!recentlyResent"
                            color="primary"
                            block
                            @click="resendVerification"
                            :loading="emailPending"
                        >
                            Resend Email Verification
                        </v-btn>

                        <div
                            v-else
                        >
                            We just sent you the verification email again! This can take some time to get sent so don't request a new one too quickly. Thanks :)
                        </div>

                        <v-btn
                            class="mt-4"
                            color="secondary"
                            @click="editEmail = true; pendingEmail = $store.state.currentUser.email;"
                            block
                        >
                            Wrong Email
                        </v-btn>

                        <v-btn
                            class="mt-4"
                            color="success"
                            block
                            :loading="checkPending"
                            @click="recheckVerification"
                            :disabled="stillNoVerify"
                        >
                            Check Again
                        </v-btn>

                        <v-btn
                            class="mt-4"
                            x-small
                            color="error"
                            :disabled="secondsLeftToHide > 0"
                            @click="showPopup = false"
                        >
                            Maybe Later <span v-if="secondsLeftToHide > 0">&nbsp;({{secondsLeftToHide}})</span>
                        </v-btn>
                    </div>

                    <v-snackbar v-model="verificationErr" :timeout="5000" color="error">
                        Failed to send a verification email. Please try again later.
                    </v-snackbar>
                </div>
            </template>

            <v-card-text v-else>
                <div>
                    Make sure you type the correct email address this time :)
                </div>

                <div class="d-flex align-center mt-2">
                    <v-text-field
                        v-model="pendingEmail"
                        filled
                        hide-details
                        single-line
                        dense
                    >
                    </v-text-field>

                    <v-btn
                        icon
                        class="ml-2"
                        color="success"
                        @click="finishEditEmail"
                        x-small
                        :loading="editInProgress"
                    >
                        <v-icon>
                            mdi-check-circle
                        </v-icon>
                    </v-btn>

                    <v-btn
                        icon
                        class="ml-2"
                        color="error"
                        @click="editEmail = false"
                        x-small
                    >
                        <v-icon>
                            mdi-close-circle
                        </v-icon>
                    </v-btn>
                </div>

                <v-snackbar
                    v-model="editFailure"
                    :timeout="5000"
                    color="error"
                >
                    Failed edit your user profile. Please try again later.
                </v-snackbar>
            </v-card-text>
        </v-card>
    </v-dialog>
</template>

<script lang="ts">
import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import ReferralLink from '@client/vue/utility/squadov/ReferralLink.vue'
import { apiClient, ApiData } from '@client/js/api'
import { getSquadOVUser, SquadOVUser } from '@client/js/squadov/user'

const MINIMUM_SECONDS_TO_DISPLAY: number = 5
const PAGES_PER_POPUP: number = 7

@Component({
    components: {
        ReferralLink,
    }
})
export default class VerifyEmailPopUp extends Vue {
    showPopup: boolean = false
    minimumDisplayTimer: number = 0

    editEmail: boolean = false
    editInProgress: boolean = false
    pendingEmail: string = ''
    editFailure: boolean = false

    recentlyResent: boolean = false
    checkPending: boolean = false
    stillNoVerify: boolean = false

    emailPending: boolean = false
    verificationErr: boolean = false

    get secondsLeftToHide(): number {
        return Math.max(0, MINIMUM_SECONDS_TO_DISPLAY - this.minimumDisplayTimer)
    }

    // We want to show the popup every once in awhile while the user is navigating through the app.
    // So we just count as the user navigates through different pages and once they go above a certain number
    // of pages, POP UP. VERIFY YOUR FUCKING EMAIL JESUS CHRIST HOW IS THIS SO HARD.
    unverifiedPageCounter: number = 0

    @Watch('unverifiedPageCounter')
    @Watch('$store.state.currentUser')
    onPageCounterChange() {
        if (this.unverifiedPageCounter == 0 && !!this.$store.state.currentUser && !this.$store.state.currentUser.verified) {
            this.showPopup = true
            this.minimumDisplayTimer = 0
        }
    }

    @Watch('$route')
    onRouteChange() {
        if (!!this.$store.state.currentUser && !this.$store.state.currentUser.verified) {
            this.unverifiedPageCounter += 1
            this.unverifiedPageCounter = this.unverifiedPageCounter % PAGES_PER_POPUP
        }
    }

    mounted() {
        this.onPageCounterChange()
        setInterval(() => {
            this.minimumDisplayTimer += 1
        }, 1000)
    }

    onResendVerification() {
        this.recentlyResent = true
        setTimeout(() => {
            this.recentlyResent = false
        }, 10000)
    }

    resendVerification() {
        this.emailPending = true
        apiClient
            .resendVerification()
            .then(() => {
                this.onResendVerification()
            })
            .catch((err: any) => {
                console.error('Failed to resend verification: ', err)
                this.verificationErr = false
            })
            .finally(() => {
                this.emailPending = false
            })
    }

    finishEditEmail() {
        this.editInProgress = true
        apiClient.editMyEmail(this.pendingEmail).then(() => {
            if (!!this.$store.state.currentUser) {
                let newUser = JSON.parse(JSON.stringify(this.$store.state.currentUser))
                newUser.email = this.pendingEmail
                newUser.verified = false
                this.$store.commit('setUser', newUser)
                this.onResendVerification()
            }
            this.pendingEmail = ''
            this.editEmail = false
        }).catch((err: any) => {
            console.error('Failed to edit email: ', err)
            this.editFailure = true
        }).finally(() => {
            this.editInProgress = false
        })
    }

    @Watch('$store.state.currentUser')
    resyncVerificationStatus() {
        if (this.showPopup && !!this.$store.state.currentUser?.verified) {
            this.showPopup = false
        }
    }

    recheckVerification() {
        if (!this.$store.state.currentUser) {
            return
        }
        this.checkPending = true
        getSquadOVUser(this.$store.state.currentUser.id).then((resp: ApiData<SquadOVUser>) => {
            this.$store.commit('setUser', resp.data)
            this.stillNoVerify = !this.$store.state.currentUser.verified
            if (this.stillNoVerify) {
                setTimeout(() => {
                    this.stillNoVerify = false
                }, 2000)
            }
        }).catch((err: any) => {
            console.error('Failed to refresh squadovuser : ', err)
        }).finally(() => {
            this.checkPending = false
        })
    }
}
</script>

<style scoped>

.button-actions {
    max-width: 60%;
}

</style>