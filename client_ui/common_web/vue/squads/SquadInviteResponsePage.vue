<template>
    <div class="d-flex flex-column justify-center align-center full-parent-height full-width">
        <div v-if="!success" class="text-h6">
            Your response is being recorded...one moment...
        </div>

        <v-progress-circular v-if="!success" class="my-2" indeterminate size="64"></v-progress-circular>
        <div class="text-h6" v-else>
            You've successfully responded to the invite. Redirecting you in {{ redirectSeconds }} seconds...
        </div>

        <v-snackbar
            v-model="showError"
            :timeout="5000"
            color="error"
        >
            Something went wrong responding to this squad invite. If you are currently logged in, double check to ensure that you are logged in on the correct account. Otherwise, please submit a bug report and try refreshing the page!
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
export default class SquadInviteResponsePage extends Vue {
    @Prop({required: true})
    inviteUuid!: string

    @Prop({required: true})
    squadId!: number

    @Prop({required: true})
    action!: string

    @Prop({type: Boolean, required: true})
    isUser!: boolean

    @Prop({required: true})
    sig!: string

    showError: boolean = false
    success: boolean = false
    redirectTime: number = 5000

    get redirectSeconds(): string {
        return (this.redirectTime / 1000).toFixed(0)
    }

    redirect() {
        this.$router.replace({
            name: pi.DashboardPageId,
        })
    }

    onSuccess() {
        this.success = true
        setTimeout(() => {
            this.redirectTime -= 1000
            if (this.redirectTime <= 0) {
                this.redirect()
            } else {
                this.onSuccess()
            }
        }, 1000)
    }

    onError() {
        this.showError = true
        this.success = false
    }

    respond() {
        // Note that we must redirect the user to the registration page if the server tells us
        // they weren't a user already.
        if (!this.isUser) {
            if (this.action === 'accept') {
                this.$router.replace({
                    name: pi.RegisterPageId,
                    query: {
                        inviteUuid: this.inviteUuid,
                        squadId: this.squadId.toString(),
                        sig: this.sig,
                    }
                })
            } else {
                apiClient.publicRejectSquadInvite(this.squadId, this.inviteUuid, this.sig).then(() => { this.onSuccess() }).catch(() => { this.onError() })
            }
        } else {
            if (this.action === 'accept') {
                apiClient.publicAcceptSquadInvite(this.squadId, this.inviteUuid, this.sig).then(() => { this.onSuccess() }).catch(() => { this.onError() })
            } else {
                apiClient.publicRejectSquadInvite(this.squadId, this.inviteUuid, this.sig).then(() => { this.onSuccess() }).catch(() => { this.onError() })
            }
        }
    }

    mounted() {
        this.respond()
    }
}

</script>