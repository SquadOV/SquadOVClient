<template>
    <v-tabs>
        <v-tab>
            Matches
        </v-tab>

        <v-tab-item>
            <user-match-watchlist
                :user-id="$store.state.currentUser.id"
            >
            </user-match-watchlist>
        </v-tab-item>

        <v-tab>
            Clips
        </v-tab>

        <v-tab-item>
            <user-clip-watchlist
                :user-id="$store.state.currentUser.id"
            >
            </user-clip-watchlist>
        </v-tab-item>
    </v-tabs>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'

import UserClipWatchlist from '@client/vue/profile/UserClipWatchlist.vue'
import UserMatchWatchlist from '@client/vue/profile/UserMatchWatchlist.vue'

@Component({
    components: {
        UserClipWatchlist,
        UserMatchWatchlist
    }
})
export default class UserWatchlistPage extends Vue {
    
}

</script>