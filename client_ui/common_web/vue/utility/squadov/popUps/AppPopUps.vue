<template>
    <div width="100%">
        <invite-friends-pop-up></invite-friends-pop-up>
    </div>
</template>

<script lang="ts">
import Vue from 'vue'
import Component from 'vue-class-component'
import InviteFriendsPopUp from '@client/vue/utility/squadov/popUps/InviteFriendsPopUp.vue'

@Component({
    components: {
        InviteFriendsPopUp,
    },
})
export default class AppPopUps extends Vue {

}
</script>