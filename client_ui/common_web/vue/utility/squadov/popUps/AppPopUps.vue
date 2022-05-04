<template>
    <div width="100%">
        <invite-friends-pop-up></invite-friends-pop-up>
        <verify-email-pop-up></verify-email-pop-up>
    </div>
</template>

<script lang="ts">
import Vue from 'vue'
import Component from 'vue-class-component'
import InviteFriendsPopUp from '@client/vue/utility/squadov/popUps/InviteFriendsPopUp.vue'
import VerifyEmailPopUp from '@client/vue/utility/squadov/popUps/VerifyEmailPopUp.vue'

@Component({
    components: {
        InviteFriendsPopUp,
        VerifyEmailPopUp,
    },
})
export default class AppPopUps extends Vue {

}
</script>