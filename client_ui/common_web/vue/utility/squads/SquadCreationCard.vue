<template>
    <v-card>
        <v-card-title>
            New Squad
        </v-card-title>

        <v-divider></v-divider>

        <v-form class="ma-4">
            <v-text-field
                v-model="newSquadName"
                label="Squad Name"
                required
                filled
            >
            </v-text-field>
        </v-form>

        <v-card-actions>
            <v-btn color="error" @click="cancelCreate" :loading="createPending">
                Cancel
            </v-btn>

            <v-spacer></v-spacer>

            <v-btn color="success" @click="performCreate" :loading="createPending">
                Create
            </v-btn>
        </v-card-actions>

        <v-snackbar
            v-model="showHideCreateSquadError"
            :timeout="5000"
            color="error"
        >
            Failed to create squad. Please try again.
        </v-snackbar>
    </v-card>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { apiClient } from '@client/js/api'

@Component
export default class SquadCreationCard extends Vue {
    newSquadName: string = ''
    createPending: boolean = false
    showHideCreateSquadError: boolean = false

    mounted() {
        this.newSquadName = ''
    }

    cancelCreate() {
        this.newSquadName = ''
        this.$emit('cancel-new-squad')
    }

    performCreate() {
        this.createPending = true
        apiClient.createSquad(this.newSquadName, this.$store.state.currentUser.username).then(() => {
            this.$emit('on-new-squad', this.newSquadName)
        }).catch((err: any) => {
            console.log('Failed to create squad: ', err)
            this.showHideCreateSquadError = true
        }).finally(() => {
            this.createPending = false
        })
    }
}

</script>