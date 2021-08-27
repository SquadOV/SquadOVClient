<template>
    <div class="d-flex flex-column justify-center align-center full-parent-height full-width">
        <div class="text-h6">
            We're working on syncing your Riot account with SquadOV. This might take a few seconds...
        </div>

        <v-progress-circular class="my-2" v-if="!showSuccess" indeterminate size="64"></v-progress-circular>

        <template v-else>
            <div class="my-2 long-text">
                Success (it still might take a few more seconds for you to see your account information)! Redirecting you back to SquadOV in {{ redirectCounter.toFixed(0) }} seconds...
            </div>

            <v-btn
                @click="redirect"
                color="success"
            >
                Redirect
            </v-btn>
        </template>

        <v-snackbar
            v-model="showError"
            :timeout="5000"
            color="error"
        >
            Something went wrong when authenticating your Riot account. Please submit a bug report and try again!
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { apiClient } from '@client/js/api'
import * as pi from '@client/js/pages'

@Component
export default class RsoOauthHandler extends Vue {
    @Prop({required: true})
    state!: string

    @Prop({required: true})
    code!: string

    showError: boolean = false
    showSuccess: boolean = false
    redirectCounter: number = 5

    redirect() {
        this.$router.replace({
            name: pi.DashboardPageId,
        })
    }
    
    startRedirectCountdown() {
        if (this.redirectCounter <= 0) {
            this.redirect()
        } else {
            setTimeout(() => {
                this.redirectCounter -= 1
                this.startRedirectCountdown()
            }, 1000)
        }
    }

    mounted() {
        apiClient.submitRsoAuthorization(this.code, this.state).then(() => {
            this.showSuccess = true
            this.startRedirectCountdown()
        }).catch((err: any) => {
            console.error('Failed to submit RSO authorization code: ', err)
            this.showError = true
        })
    }
}

</script>