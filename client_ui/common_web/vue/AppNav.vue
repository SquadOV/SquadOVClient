<template>
    <v-app-bar
        dense
    >
        <v-toolbar-title class="mr-4">
            <router-link :to="homeTo">
                SquadOV 
            </router-link>
        </v-toolbar-title>

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
                        <v-list-item :key="c.name" :to="c.to" v-if="!c.disabled">
                            <v-list-item-title>{{ c.name }}</v-list-item-title>
                        </v-list-item>
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
            <v-menu bottom offset-y>
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
        </template>

        <template v-else>
            <v-btn color="success" :to="registerTo">
                Register
            </v-btn>
        </template>
    </v-app-bar>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import * as pi from '@client/js/pages'
import { apiClient, ApiData } from '@client/js/api'
import { clearSessionCookie } from '@client/js/session'

/// #if DESKTOP
import { ipcRenderer } from 'electron'
/// #endif

import { NotificationSummary } from '@client/js/squadov/notification'

@Component
export default class AppNav extends Vue {
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
                        disabled: !this.$store.state.features.enableLol
                    },
                    {
                        name: 'Teamfight Tactics',
                        to: {
                            name: pi.TftLogPageId,
                            params: this.gameLogParams,
                            query: this.gameLogQuery
                        },
                        disabled: !this.$store.state.features.enableTft
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

    @Watch('$route')
    @Watch('isLoggedIn')
    refreshNotifications() {
        if (!this.isLoggedIn) {
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
}

</script>

<style scoped>

.inner-badge {
    margin-top: 0px !important;
}

</style>