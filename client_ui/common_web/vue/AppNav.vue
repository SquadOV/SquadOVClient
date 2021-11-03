<template>
    <v-app-bar
        v-if="isLoggedIn"
        dense
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
                <template v-slot:activator="{ on, attrs }">
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
                            <v-tooltip
                                :key="c.name"
                                :disabled="!c.disableOnWeb && !isDesktop"
                                bottom
                            >
                                <template v-slot:activator="{ on }">
                                    <div v-on="on">
                                        <v-list-item
                                            :key="c.name"
                                            :to="c.to"
                                            :disabled="c.disableOnWeb && !isDesktop"
                                            v-if="!c.link"
                                        >
                                            <v-badge
                                                inline
                                                right
                                                :value="c.notifications > 0"
                                                :content="`${c.notifications}`"
                                                color="error"
                                            >
                                                <v-list-item-title>{{
                                                    c.name
                                                }}</v-list-item-title>
                                            </v-badge>
                                        </v-list-item>

                                        <v-list-item
                                            :key="c.name"
                                            @click="goToLink(c.to)"
                                            v-else
                                        >
                                            <v-badge
                                                inline
                                                right
                                                :value="c.notifications > 0"
                                                :content="`${c.notifications}`"
                                                color="error"
                                            >
                                                <v-list-item-title>{{
                                                    c.name
                                                }}</v-list-item-title>
                                            </v-badge>
                                        </v-list-item>
                                    </div>
                                </template>
                                <span> Only available in the desktop application. </span>
                            </v-tooltip>
                        </template>
                    </template>
                </v-list>
            </v-menu>
        </template>

        <v-spacer></v-spacer>

        <download-button v-if="!isDesktop" class="mr-2"></download-button>

        <v-menu
            bottom
            left
            offset-y
            :close-on-content-click="false"
            class="mr-4"
        >
            <template v-slot:activator="{ on, attrs }">
                <v-btn icon v-bind="attrs" v-on="on">
                    <v-icon> mdi-history </v-icon>
                </v-btn>
            </template>

            <div class="news-div">
                <news-display></news-display>
            </div>
        </v-menu>

        <template v-if="isLoggedIn">
            <v-toolbar-title>
                <v-tooltip :disabled="isDesktop" bottom>
                    <template v-slot:activator="{ on }">
                        <div v-on="on">
                            <v-btn
                                icon
                                :to="settingsTo"
                                :disabled="!isDesktop"
                            >
                                <v-icon> mdi-cog </v-icon>
                            </v-btn>
                        </div>
                    </template>
                    <span>
                        Go to the Desktop Application to modify settings.
                    </span>
                </v-tooltip>
            </v-toolbar-title>

            <v-menu bottom left offset-y class="mr-4">
                <template v-slot:activator="{ on, attrs }">
                    <v-btn icon v-bind="attrs" v-on="on">
                        <v-icon> mdi-account </v-icon>
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
            <v-btn color="success" :to="registerTo"> Register </v-btn>
        </template>
    </v-app-bar>
</template>

<script lang="ts">

import Component, { mixins } from 'vue-class-component'
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

@Component({
    components: {
        DownloadButton,
        NewsDisplay,
    }
})
export default class AppNav extends mixins(CommonComponent) {
    notifications: NotificationSummary | null = null

    get isDesktop(): boolean {
/// #if DESKTOP
        return true
/// #else
        return false
/// #endif
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

    get gameLogQuery(): any {
        return this.cleanQuery
    }

    get menuItems(): any[] {
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
                    {
                        name: 'Local',
                        to: {
                            name: pi.LocalStoragePageId,
                        },
                        disableOnWeb: true,
                    },
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
                        to: 'https://discord.gg/6Rj5jCVDeC',
                        link: true,
                    },
                ]
            }
        ]
    }

    get homeTo(): any {
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

    logout() {
        apiClient.logout().then(() => {
        }).catch((err: any) => {
            console.error('Failed to logout: {}', err)
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
            console.error('Failed to obtain notifications: ', err)
        })
    }

    mounted() {
        this.refreshNotifications()
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

.news-div {
    max-height: 70vh;
    width: 33vw;
    background-color: #121212;
    border: 1px white solid;
    overflow: auto;
}
</style>