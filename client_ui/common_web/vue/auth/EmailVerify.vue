<template>
    <v-container fluid class="full-parent-height">
        <v-row class="full-parent-height" justify="center" align="center">
            <v-col cols="8">
                <div class="text-h5 mb-4">
                    Almost there! Check your inbox for your verification email.
                    Click the "I'm Verified!" button once you have verified your email address.
                </div>

                <v-row>
                    <v-col cols="6">
                        <v-btn
                            block
                            @click="resendVerification"
                            color="warning"
                            :loading="resendInProgress"
                        >
                            Resend Verification Email
                        </v-btn>
                    </v-col>

                    <v-col cols="6">
                        <v-btn
                            block
                            @click="checkVerification"
                            color="success"
                            :loading="checkInProgress"
                        >
                            I'm Verified!
                        </v-btn>
                    </v-col>
                </v-row>
            </v-col>
        </v-row>

        <v-snackbar
            v-model="showHideSent"
            :timeout="5000"
            color="success"
        >
            Verification email sent! Check your inbox.
        </v-snackbar>

        <v-snackbar
            v-model="showHideError"
            :timeout="5000"
            color="error"
        >
            {{ errorMessage }}
        </v-snackbar>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { apiClient, ApiData, CheckVerificationOutput } from '@client/js/api'
import * as pi from '@client/js/pages'

/// #if DESKTOP
import { ipcRenderer } from 'electron'
/// #endif

@Component
export default class EmailVerify extends Vue {
    @Prop({default: true})
    ipc!: boolean

    showHideSent: boolean = false
    showHideError: boolean = false
    errorMessage: string = ''

    resendInProgress: boolean = false
    checkInProgress: boolean = false

    resendVerification() {
        apiClient.resendVerification().then(() => {
            this.showHideSent = true
        }).catch((err : any) => {
            console.error('Resend Verification Error: ', err)
            this.showHideError = true
            this.errorMessage = `Oops! Something went wrong. Please try again.`
        })
    }

    checkVerification() {
        apiClient.checkVerification().then((resp : ApiData<CheckVerificationOutput>) => {
            if (!!resp.data.verified) {
/// #if DESKTOP
                if (this.ipc) {
                    ipcRenderer.send('finish-login')
                } else {
                    this.$router.replace({
                        name: pi.DashboardPageId,
                    })    
                }
/// #else
                this.$router.replace({
                    name: pi.DashboardPageId,
                })
/// #endif
            } else {
                this.showHideError = true
                this.errorMessage = `Oops! You haven't verified your email address. Please try resending a verification email.`
            }
        }).catch((err : any) => {
            console.error('Check Verification Error: ', err)
            this.showHideError = true
            this.errorMessage = `Oops! Something went wrong. Please try again.`
        })
    }
}

</script>