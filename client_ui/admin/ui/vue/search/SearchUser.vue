<template>
    <div class="full-width">
        <div class="d-flex align-center ma-4">
            <v-text-field
                v-model="userSearch"
                label="Id/Username/Email/Uuid"
                solo
                dense
                hide-details
            >
            </v-text-field>

            <v-btn
                class="ml-4"
                color="primary"
                :disabled="userSearch.trim() == ''"
                @click="searchUser"
                :loading="searching"
            >
                Search
            </v-btn>
        </div>

        <div v-if="!!currentUser">
            <user-display :user="currentUser"></user-display>
        </div>

        <div class="text-h2" v-else>
            No User Found
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { User, searchForUser } from '@client/ts/user'
import UserDisplay from '@client/vue/user/UserDisplay.vue'

@Component({
    components: {
        UserDisplay
    }
})
export default class SearchUser extends Vue {
    userSearch: string = ''
    searching: boolean = false
    currentUser: User | null = null

    searchUser() {
        searchForUser(this.userSearch).then((resp: User | null) => {
            this.currentUser = resp
        }).catch((err: any) => {
            console.error('Failed to get user: ', err)
        })
    }
}

</script>