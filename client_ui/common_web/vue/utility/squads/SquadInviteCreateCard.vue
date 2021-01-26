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
                :disabled="inviteUsernames.length == 0"
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

    showSuccess: boolean = false
    showError: boolean = false

    sendInvite() {
        this.invitePending = true
        apiClient.sendSquadInvite(this.squadId, this.inviteUsernames).then(() => {
            this.showSuccess = true
            this.$emit('on-send-invite')
        }).catch((err: any) => {
            console.log('Failed to send squad invites: ', err)
            this.showError = true
        }).finally(() => {
            this.invitePending = false
        })
    }
}

</script>