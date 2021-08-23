<template>
    <v-tabs>
        <v-tab>
            Matches
        </v-tab>

        <v-tab-item>
            <user-match-favorites
                :user-id="$store.state.currentUser.id"
            >
            </user-match-favorites>
        </v-tab-item>

        <v-tab>
            Clips
        </v-tab>

        <v-tab-item>
            <user-clip-favorites
                :user-id="$store.state.currentUser.id"
            >
            </user-clip-favorites>
        </v-tab-item>
    </v-tabs>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'

import UserClipFavorites from '@client/vue/profile/UserClipFavorites.vue'
import UserMatchFavorites from '@client/vue/profile/UserMatchFavorites.vue'

@Component({
    components: {
        UserClipFavorites,
        UserMatchFavorites
    }
})
export default class UserFavoritesPage extends Vue {
    
}

</script>