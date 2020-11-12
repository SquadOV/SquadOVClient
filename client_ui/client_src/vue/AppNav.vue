<template>
    <v-app-bar
        dense
    >
        <v-toolbar-title class="mr-4">
            <router-link :to="homeTo">
                SquadOV 
            </router-link>
        </v-toolbar-title>

        <v-menu bottom offset-y v-for="m in menu" :key="m.name">
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

        <v-btn @click="logout" text>
            Logout
        </v-btn>
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
    menu : any[] = [
        {
            name: 'Game Logs',
            children: [
                {
                    name: 'Valorant',
                    to: {
                        name: pi.ValorantLogPageId,
                        params: this.$route.params
                    },
                },
                {
                    name: 'Aim Lab',
                    to: {
                        name: pi.AimlabLogPageId,
                        params: this.$route.params
                    },
                },
                {
                    name: 'Hearthstone',
                    to: {
                        name: pi.HearthstoneLogPageId,
                        params: this.$route.params
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
                        params: this.$route.params
                    },
                },
            ]
        },
    ]

    get homeTo() : any {
        return {
            name: pi.DashboardPageId,
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