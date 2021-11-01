<template>
    <div>
        <v-dialog width="500" v-model="showInviteFriendsNotification">
            <v-card>
                <v-card-title style="font-size: x-large">
                    Invite your Friends!
                </v-card-title>
                <v-card-subtitle style="font-size: x-small">
                    Tell them to join the dark side.
                </v-card-subtitle>
                <v-card-text class="pop-up-vcard-text">
                    SquadOV is better with friends. Play, improve, and have fun
                    as a squad!
                </v-card-text>
                <v-card-text
                    class="pop-up-vcard-text"
                    style="font-weight: italic; font-style: italic"
                >
                    Send your friends your referral link below:
                </v-card-text>
                <referral-link class="referral-link"></referral-link>
                <v-card-actions>
                    <a class="do-not-show-button" @click="muteFuturePopUp()"
                        >Don't Show Again</a
                    >
                    <v-spacer></v-spacer>
                    <v-btn @click="closeNotification()" small color="error">
                        Close
                    </v-btn>
                </v-card-actions>
            </v-card>
        </v-dialog>
    </div>
</template>

<script lang="ts">
import Vue from 'vue'
import Component from 'vue-class-component'
import { apiClient, ApiData } from '@client/js/api'
import '@mdi/font/css/materialdesignicons.css'
import { Watch } from 'vue-property-decorator'
import ReferralLink from '@client/vue/utility/squadov/ReferralLink.vue'

@Component({
    components: {
        ReferralLink,
    }
})
export default class InviteFriendsPopUp extends Vue {
    showInviteFriendsNotification: boolean = false

    get isInviteFriendsNotificationMuted() {
        return this.$store.state.muteInviteFriendsPopUp
    }

    get isLoggedIn(): boolean {
        return !!this.$store.state.currentUser
    }

    closeNotification() {
        this.showInviteFriendsNotification = false
    }

    muteFuturePopUp() {
        this.showInviteFriendsNotification = false
        this.$store.commit('muteInviteFriendsPopUp', true)
    }

    @Watch('$store.state.displayInviteFriendPopUp')
    @Watch('isLoggedIn')
    checkLoginStatus() {
        if (this.isLoggedIn && !this.isInviteFriendsNotificationMuted && this.$store.state.displayInviteFriendPopUp) {
            this.showInviteFriendsNotification = true
        }
    }

    mounted() {
        this.checkLoginStatus()
    }
}
</script>
<style scoped>
.do-not-show-button {
    font-size: x-small;
}
.pop-up-vcard-text {
    font-size: large;
}
.referral-link {
    padding-left: 20px;
    padding-right: 20px;
    padding-bottom: 20px;
}
</style>