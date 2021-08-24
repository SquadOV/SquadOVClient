<template>
    <v-app-bar
        dense
        :extension-height="extensionHeight"
    >
        <v-btn
            text
            exact
            :to="homeTo"
        >
            Home
        </v-btn>

        <template v-if="isLoggedIn">
            <v-menu bottom offset-y v-for="m in menuItems" :key="m.name">
                <template v-slot:activator="{on, attrs}">
                    <v-btn text v-bind="attrs" v-on="on" :to="m.to" exact>
                        <v-badge
                            inline
                            left
                            :value="m.notifications > 0"
                            :content="`${m.notifications}`"
                            color="error"
                        >
                            {{ m.name }}
                            <v-icon small v-if="!!m.children">
                                mdi-chevron-down
                            </v-icon> 
                        </v-badge>
                    </v-btn>
                </template>

                <v-list dense v-if="!!m.children">
                    <template v-for="c in m.children">
                        <template v-if="!c.disabled">
                            <v-list-item :key="c.name" :to="c.to" v-if="!c.link">
                                <v-badge
                                    inline
                                    right
                                    :value="c.notifications > 0"
                                    :content="`${c.notifications}`"
                                    color="error"
                                >
                                    <v-list-item-title>{{ c.name }}</v-list-item-title>
                                </v-badge>
                            </v-list-item>

                            <v-list-item :key="c.name" @click="goToLink(c.to)" v-else>
                                <v-badge
                                    inline
                                    right
                                    :value="c.notifications > 0"
                                    :content="`${c.notifications}`"
                                    color="error"
                                >
                                    <v-list-item-title>{{ c.name }}</v-list-item-title>
                                </v-badge>
                            </v-list-item>
                        </template>
                    </template>
                </v-list>
            </v-menu>
        </template>

        <v-spacer></v-spacer>

        <download-button v-if="!isDesktop" class="mr-2"></download-button>

        <v-menu bottom left offset-y :close-on-content-click="false" class="mr-4">
            <template v-slot:activator="{on, attrs}">
                <v-btn icon v-bind="attrs" v-on="on">
                    <v-icon>
                        mdi-history
                    </v-icon>
                </v-btn>
            </template>

            <div class="news-div">
                <news-display></news-display>
            </div>
        </v-menu>

        <template v-if="isLoggedIn">
            <v-toolbar-title>
                <v-btn icon :to="settingsTo" v-if="settingsEnabled">
                    <v-icon>
                        mdi-cog
                    </v-icon>
                </v-btn>
            </v-toolbar-title>

            <v-menu bottom left offset-y class="mr-4">
                <template v-slot:activator="{on, attrs}">
                    <v-btn icon v-bind="attrs" v-on="on">
                        <v-icon>
                            mdi-account
                        </v-icon>
                    </v-btn>
                </template>

                <v-list dense>
                    <v-list-item :to="profileTo">
                        <v-list-item-title>Profile</v-list-item-title>
                    </v-list-item>

                    <v-list-item @click="logout">
                        <v-list-item-title>Logout</v-list-item-title>
                    </v-list-item>
                </v-list>
            </v-menu>
        </template>

        <template v-else>
            <v-btn color="success" :to="registerTo">
                Register
            </v-btn>
        </template>

        <template v-slot:extension>
            <v-alert
                v-model="showEmailVerification"
                class="pa-2 ma-0"
                type="warning"
                dismissible
                tile
                width="100%"
            >
                <template v-slot:prepend>
                    <v-icon>mdi-exclamation</v-icon>
                </template>

                <div class="d-flex align-center">
                    <div>
                        Your email address is unverified. Please verify your email address ({{ emailAddress }}) ASAP to prevent account deletion.
                    </div>

                    <v-spacer></v-spacer>

                    <v-btn small color="primary" @click="resendVerification" v-if="!sentVerification" :loading="emailPending">
                        Resend Email Verification
                    </v-btn>

                    <div v-else>
                        Email Sent!
                    </div>

                    <v-btn small v-if="!sentVerification" class="ml-2" color="secondary" :to="accountSettingsTo">
                        Wrong email!
                    </v-btn>

                    <v-btn
                        class="ml-2"
                        color="success"
                        small
                        icon
                        @click="recheckVerification"
                        :loading="checkPending"
                    >
                        <v-icon>
                            mdi-refresh
                        </v-icon>
                    </v-btn>
                </div>
            </v-alert>
        </template>

        <v-snackbar
            v-model="verificationErr"
            :timeout="5000"
            color="error"
        >
            Failed to send a verification email. Please try again later.
        </v-snackbar>
    </v-app-bar>
</template>

<script lang="ts">

import Component, {mixins} from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import * as pi from '@client/js/pages'
import { apiClient, ApiData } from '@client/js/api'
import { clearSessionCookie } from '@client/js/session'
import { openUrlInBrowser } from '@client/js/external'
import CommonComponent from '@client/vue/CommonComponent'
import DownloadButton from '@client/vue/utility/DownloadButton.vue'
import NewsDisplay from '@client/vue/utility/squadov/NewsDisplay.vue'

/// #if DESKTOP
import { ipcRenderer } from 'electron'
/// #endif

import { NotificationSummary } from '@client/js/squadov/notification'
import { getSquadOVUser, SquadOVUser } from '@client/js/squadov/user'

@Component({
    components: {
        DownloadButton,
        NewsDisplay,
    }
})
export default class AppNav extends mixins(CommonComponent) {
    notifications: NotificationSummary | null = null
    showEmailVerification: boolean = false
    sentVerification: boolean = false
    emailPending: boolean = false
    verificationErr: boolean = false
    checkPending: boolean = false

    get emailAddress(): string {
        return this.$store.state.currentUser.email
    }

    
    recheckVerification() {
        this.checkPending = true
        getSquadOVUser(this.$store.state.currentUser.id).then((resp: ApiData<SquadOVUser>) => {
            this.$store.commit('setUser' , resp.data)
        }).catch((err: any) => {
            console.log('Failed to refresh squadovuser : ', err)
        }).finally(() => {
            this.checkPending = false
        })
    }

    resendVerification() {
        this.emailPending = true
        apiClient.resendVerification().then(() => {
            this.sentVerification = true
            setTimeout(() => {
                this.showEmailVerification = false
            }, 5000)
        }).catch((err: any) => {
            console.log('Failed to resend verification: ', err)
            this.verificationErr = false
        }).finally(() => {
            this.emailPending = false
        })
    }

    get extensionHeight(): number {
        if (this.showEmailVerification) {
            return 40
        } else {
            return 0
        }
    }

    get isDesktop(): boolean {
/// #if DESKTOP
        return true
/// #else
        return false
/// #endif
    }

    get downloadUrl(): string {
        return 'https://us-central1.content.squadov.gg/builds/SquadOV.exe'
    }

    get registerTo(): any { 
        return {
            name: pi.RegisterPageId
        }
    }

    get isLoggedIn(): boolean {
        return !!this.$store.state.currentUser
    }

    get totalSquadInvites(): number {
        if (!this.notifications) {
            return 0
        }
        return this.notifications.numSquadInvites
    }

    get gameLogParams(): any {
        let params = this.$route.params
        if (params.userId !== undefined) {
            return params
        } else {
            return {
                userId: this.$store.state.currentUser.id
            }
        }
    }

    get gameLogQuery() : any {
        return this.$route.query
    }

    get menuItems(): any [] {
        return [
            {
                name: 'Game Logs',
                to: {
                    name: pi.LogPageId,
                    params: this.gameLogParams,
                    query: this.gameLogQuery
                },
            },
            {
                name: 'Social',
                children: [
                    {
                        name: 'Squads',
                        to: this.squadsTo,
                        notifications: this.totalSquadInvites,
                    },
                    /*
                    {
                        name: 'Communities',
                        to: {
                            name: pi.CommunityHomePageId,
                        },
                    },
                    */
                ],
                notifications: this.totalSquadInvites,
            },
            {
                name: 'Performance',
                children: [
                    {
                        name: 'Visualization',
                        to: {
                            name: pi.VizStatsPageId,
                        },
                    },
                ]
            },
            {
                name: 'Library',
                children: [
                    {
                        name: 'Favorites',
                        to: {
                            name: pi.FavoritesPageId,
                        },
                    },
                    {
                        name: 'Watchlist',
                        to: {
                            name: pi.WatchlistPageId,
                        },
                    },
                    {
                        name: 'Clips',
                        to: {
                            name: pi.ClipLibraryPageId,
                        },
                    },
///#if DESKTOP
                    {
                        name: 'Local',
                        to: {
                            name: pi.LocalStoragePageId,
                        },
                    },
///#endif
                ]
            },
            {
                name: 'Support',
                children: [
                    {
                        name: 'Website',
                        to: 'https://support.squadov.gg',
                        link: true,
                    },
                    {
                        name: 'Setup Wizard',
                        to: {
                            name: pi.SetupWizardPageId
                        }
                    },
                    {
                        name: 'Discord',
                        to: 'https://discord.gg/jVyfv9ZHHV',
                        link: true,
                    },
                ]
            }
        ]
    }

    get homeTo() : any {
        return {
            name: pi.DashboardPageId,
        }
    }

    get currentUserName(): string {
        if (!this.$store.state.currentUser) {
            return 'Unknown'
        }

        return this.$store.state.currentUser.username
    }

    get accountSettingsTo(): any {
        return {
            name: pi.SettingsPageId,
            query: {
                inputTab: 2
            }
        }
    }

    get profileTo(): any {
        return {
            name: pi.UserProfileIdPageId,
            params: {
                userId: this.$store.state.currentUser!.id
            }
        }
    }

    get squadsTo(): any {
        return {
            name: pi.UserSquadsPageId,
            params: {
                userId: this.$store.state.currentUser!.id
            }
        }
    }

    get settingsTo(): any {
        return {
            name: pi.SettingsPageId,
        }
    }

    get settingsEnabled(): boolean {
/// #if DESKTOP
        return true
/// #else  
        return false
/// #endif
    }

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

    @Watch('$route.fullPath')
    @Watch('isLoggedIn')
    @Watch('isActive')
    refreshNotifications() {
        if (!this.isLoggedIn || !this.isActive) {
            return
        }

        apiClient.getNotificationSummary().then((resp: ApiData<NotificationSummary>) => {
            this.notifications = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain notifications: ', err)
        })
    }

    mounted() {
        this.refreshNotifications()
        this.resyncVerificationStatus()
    }

    @Watch('$store.state.currentUser')
    resyncVerificationStatus() {
        if (this.isLoggedIn) {
            this.showEmailVerification = !this.$store.state.currentUser.verified
        }
    }

    goToLink(url: string) {
        openUrlInBrowser(url)
    }
}

</script>

<style scoped>

.inner-badge {
    margin-top: 0px !important;
}

>>>.v-toolbar__extension {
    padding: 0;
    margin: 0;
}

.news-div {
    max-height: 70vh;
    width: 33vw;
    background-color: #121212;
    border: 1px white solid;
    overflow: auto;
}

</style>