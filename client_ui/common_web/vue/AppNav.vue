<template>
    <v-app-bar
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
                <template v-slot:activator="{on, attrs}">
                    <v-btn text v-bind="attrs" v-on="on">
                        {{ m.name }}
                        <v-icon small>
                            mdi-chevron-down
                        </v-icon> 
                    </v-btn>
                </template>

                <v-list dense>
                    <template v-for="c in m.children">
                        <template v-if="!c.disabled">
                            <v-list-item :key="c.name" :to="c.to" v-if="!c.link">
                                <v-list-item-title>{{ c.name }}</v-list-item-title>
                            </v-list-item>

                            <v-list-item :key="c.name" @click="goToLink(c.to)" v-else>
                                <v-list-item-title>{{ c.name }}</v-list-item-title>
                            </v-list-item>
                        </template>
                    </template>
                </v-list>
            </v-menu>
        </template>

        <v-spacer></v-spacer>

        <v-btn v-if="!isDesktop" class="mr-2" color="primary" :href="downloadUrl">
            <v-icon class="mr-1">
                mdi-microsoft-windows
            </v-icon>
            Windows Client
        </v-btn>

        <template v-if="isLoggedIn">
            <v-menu bottom offset-y class="mr-4">
                <template v-slot:activator="{on, attrs}">
                    <v-btn text v-bind="attrs" v-on="on">
                        <v-badge
                            inline
                            left
                            :value="totalNotifications > 0"
                            :content="`${totalNotifications}`"
                            color="error"
                        >
                            {{ currentUserName }}
                            <v-icon small>
                                mdi-chevron-down
                            </v-icon> 
                        </v-badge>
                    </v-btn>
                </template>

                <v-list dense>
                    <v-list-item :to="profileTo">
                        <v-list-item-title>Profile</v-list-item-title>
                    </v-list-item>

                    <v-list-item :to="squadsTo">
                        <v-list-item-title>
                            <v-badge
                                class="inner-badge"
                                inline
                                :value="totalSquadInvites > 0"
                                :content="`${totalSquadInvites}`"
                                color="error"
                            >
                                Squads
                            </v-badge>
                        </v-list-item-title>
                    </v-list-item>

                    <v-list-item @click="logout">
                        <v-list-item-title>Logout</v-list-item-title>
                    </v-list-item>
                </v-list>
            </v-menu>

            <v-toolbar-title>
                <v-btn icon :to="settingsTo" v-if="settingsEnabled">
                    <v-icon>
                        mdi-cog
                    </v-icon>
                </v-btn>
            </v-toolbar-title>
        </template>

        <template v-else>
            <v-btn color="success" :to="registerTo">
                Register
            </v-btn>
        </template>
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

/// #if DESKTOP
import { ipcRenderer } from 'electron'
/// #endif

import { NotificationSummary } from '@client/js/squadov/notification'

@Component
export default class AppNav extends mixins(CommonComponent) {
    notifications: NotificationSummary | null = null

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

    get totalNotifications(): number {
        return this.totalSquadInvites
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
                children: [
                    {
                        name: 'Aim Lab',
                        to: {
                            name: pi.AimlabLogPageId,
                            params: this.gameLogParams,
                            query: this.gameLogQuery
                        },
                    },
                    {
                        name: 'CS:GO',
                        to: {
                            name: pi.CsgoLogPageId,
                            params: this.gameLogParams,
                            query: this.gameLogQuery
                        },
                    },
                    {
                        name: 'Hearthstone',
                        to: {
                            name: pi.HearthstoneLogPageId,
                            params: this.gameLogParams,
                            query: this.gameLogQuery
                        },
                    },
                    {
                        name: 'League of Legends',
                        to: {
                            name: pi.LolLogPageId,
                            params: this.gameLogParams,
                            query: this.gameLogQuery
                        },
                    },
                    {
                        name: 'Teamfight Tactics',
                        to: {
                            name: pi.TftLogPageId,
                            params: this.gameLogParams,
                            query: this.gameLogQuery
                        },
                    },
                    {
                        name: 'Valorant',
                        to: {
                            name: pi.ValorantLogPageId,
                            params: this.gameLogParams,
                            query: this.gameLogQuery
                        },
                    },
                    {
                        name: 'World of Warcraft',
                        to: {
                            name: pi.WowLogPageId,
                            params: this.gameLogParams,
                            query: this.gameLogQuery
                        },
                    },
                ]
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
                            name: pi.UserProfilePageId,
                            params: {
                                userId: this.$store.state.currentUser!.id,
                            },
                            query: {
                                tab: 1,
                                matchTab: 0,
                            }
                        },
                    },
                    {
                        name: 'Clips',
                        to: {
                            name: pi.ClipLibraryPageId,
                        },
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

    get profileTo(): any {
        return {
            name: pi.UserProfilePageId,
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

</style>