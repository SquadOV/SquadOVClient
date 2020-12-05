<template>
    <v-app-bar
        dense
    >
        <v-toolbar-title class="mr-4">
            <router-link :to="homeTo">
                SquadOV 
            </router-link>
        </v-toolbar-title>

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
                <v-list-item v-for="c in m.children" :key="c.name" :to="c.to">
                    <v-list-item-title>{{ c.name }}</v-list-item-title>
                </v-list-item>
            </v-list>
        </v-menu>

        <v-spacer></v-spacer>

        <v-menu bottom offset-y>
            <template v-slot:activator="{on, attrs}">
                <v-btn text v-bind="attrs" v-on="on">
                    {{ currentUserName }}
                    <v-icon small>
                        mdi-chevron-down
                    </v-icon> 
                </v-btn>
            </template>

            <v-list dense>
                <v-list-item :to="profileTo">
                    <v-list-item-title>Profile</v-list-item-title>
                </v-list-item>

                <v-list-item :to="squadsTo">
                    <v-list-item-title>Squads</v-list-item-title>
                </v-list-item>

                <v-list-item @click="logout">
                    <v-list-item-title>Logout</v-list-item-title>
                </v-list-item>
            </v-list>
        </v-menu>
    </v-app-bar>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import * as pi from '@client/js/pages'
import { apiClient } from '@client/js/api'
import { ipcRenderer } from 'electron'

@Component
export default class AppNav extends Vue {

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
                        name: 'Valorant',
                        to: {
                            name: pi.ValorantLogPageId,
                            params: this.gameLogParams,
                            query: this.gameLogQuery
                        },
                    },
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
            ipcRenderer.sendSync('logout')
        })
    }
}

</script>

<style scoped>

</style>