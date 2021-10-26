<template>
    <div width="100%">
        <invite-friends-pop-up
            v-if="showInviteFriendsNotification"
        ></invite-friends-pop-up>
    </div>
</template>

<script lang="ts">
import Vue from 'vue'
import Component from 'vue-class-component'
import InviteFriendsPopUp from '@client/vue/utility/squadov/popUps/InviteFriendsPopUp.vue'
import { Watch } from 'vue-property-decorator'

@Component({
    components: {
        InviteFriendsPopUp,
    },
})
export default class AppPopUps extends Vue {
    showInviteFriendsNotification: boolean = false

    get isInviteFriendsNotificationMuted() {
        if (this.$store.state.muteInviteFriendsPopUp) {
            return true
        }
        return false
    }

    get isLoggedIn(): boolean {
        return !!this.$store.state.currentUser
    }

    @Watch('$store.state.currentUser')
    checkLoginStatus() {
        if (this.isLoggedIn && !this.isInviteFriendsNotificationMuted) {
            this.showInviteFriendsNotification = true
        }
    }

    mounted() {
        this.checkLoginStatus()
    }

}
</script>