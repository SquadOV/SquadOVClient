<template>
    <div width="100%" v-if="isLoggedIn">
        <v-list width="100%" dense tile subheader>
            <email-verification-alert width="100%"> </email-verification-alert>
            <local-disk-space-usage-alert
                width="100%"
                v-if="isDesktop"
            ></local-disk-space-usage-alert>
            <disabled-games-alert
                width="100%"
                v-if="isDesktop"
            >
            </disabled-games-alert>

            <requires-restart-alert
                width="100%"
                v-if="isDesktop"
            >
            </requires-restart-alert>
        </v-list>
    </div>
</template>

<script lang="ts">
import Vue from 'vue'
import Component from 'vue-class-component'
import EmailVerificationAlert from '@client/vue/utility/squadov/alerts/EmailVerificationAlert.vue'
import LocalDiskSpaceUsageAlert from '@client/vue/utility/squadov/alerts/LocalDiskSpaceUsageAlert.vue'
import DisabledGamesAlert from '@client/vue/utility/squadov/alerts/DisabledGamesAlert.vue'
import RequiresRestartAlert from '@client/vue/utility/squadov/alerts/RequiresRestartAlert.vue'

@Component({
    components: {
        EmailVerificationAlert,
        LocalDiskSpaceUsageAlert,
        DisabledGamesAlert,
        RequiresRestartAlert,
    },
})
export default class AppAlerts extends Vue {

    get isLoggedIn(): boolean {
        return this.$store.state.currentUser
    }

    setSessionAlerts() {
        if (window.sessionStorage.getItem('muteLowStorageAlert') == null) {
            window.sessionStorage.setItem('muteLowStorageAlert', 'false')
        }
        if (window.sessionStorage.getItem('muteEmailAlert') == null) {
            window.sessionStorage.setItem('muteEmailAlert', 'false')
        }
    }

    created() {
        this.setSessionAlerts()
    }

    get isDesktop(): boolean {
        /// #if DESKTOP
        return true
        /// #else
        return false
        /// #endif
    }
}
</script>
