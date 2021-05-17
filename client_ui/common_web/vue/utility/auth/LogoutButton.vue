<template>
    <v-btn
        color="error"
        @click="logout"
    >
        Logout
    </v-btn>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { apiClient } from '@client/js/api'
import { clearSessionCookie } from '@client/js/session'
import * as pi from '@client/js/pages'

/// #if DESKTOP
import { ipcRenderer } from 'electron'
/// #endif

@Component
export default class LogoutButton extends Vue {
    logout() {
        apiClient.logout().then(() => {
        }).catch((err : any) => {
            console.log('Failed to logout: {}', err)
        }).finally(() => {
            // The user should feel like they logged out regardless of what happened on the server.
/// #if DESKTOP
            ipcRenderer.sendSync('logout')
/// #else
            // Delete session cookie and then do a redirect to the login page as we don't have
            // any facilities to detect this deletion.
            clearSessionCookie(this.$store)

            this.$router.replace({
                name: pi.LoginPageId,
            })
/// #endif
        })
    }
}

</script>