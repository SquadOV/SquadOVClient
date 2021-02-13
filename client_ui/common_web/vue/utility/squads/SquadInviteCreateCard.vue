<template>
    <v-card>
        <v-card-title>
            Invite Users
        </v-card-title>
        <v-divider></v-divider>

        <v-combobox
            v-model="inviteUsernames"
            clearable
            filled
            chips
            label="Usernames"
            multiple
            deletable-chips
            hide-details
        >
        </v-combobox>

        <div class="d-flex my-4 justify-center align-center">
            OR
        </div>

        <v-combobox
            v-model="inviteEmails"
            clearable
            filled
            chips
            label="Emails"
            multiple
            deletable-chips
            hide-details
        >
        </v-combobox>

        <v-card-actions>
            <v-btn color="error" @click="$emit('on-cancel-invite')">
                Cancel
            </v-btn>

            <v-spacer></v-spacer>

            <v-btn
                color="success"
                :loading="invitePending"
                @click="sendInvite"
                :disabled="totalInvites == 0"
            >
                Send
            </v-btn>
        </v-card-actions>
    </v-card>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { apiClient } from '@client/js/api' 

@Component
export default class SquadInviteCreateCard extends Vue {
    @Prop({required: true})
    squadId!: number

    invitePending: boolean = false
    inviteUsernames: string[] = []
    inviteEmails: string[] = []

    showSuccess: boolean = false
    showError: boolean = false

    get totalInvites(): number {
        return this.inviteUsernames.length + this.inviteEmails.length
    }

    clear() {
        this.inviteUsernames = []
        this.inviteEmails = []
    }

    sendInvite() {
        this.invitePending = true
        apiClient.sendSquadInvite(this.squadId, this.inviteUsernames, this.inviteEmails).then(() => {
            this.showSuccess = true
            this.$emit('on-send-invite')
            this.clear()
        }).catch((err: any) => {
            console.log('Failed to send squad invites: ', err)
            this.showError = true
        }).finally(() => {
            this.invitePending = false
        })
    }
}

</script>