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
    </v-app-bar>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import * as pi from '@client/js/pages'

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
                    },
                },
                {
                    name: 'Aim Lab',
                    to: {
                        name: pi.AimlabLogPageId,
                    },
                },
            ]
        },
        {
            name: 'Performance',
            children: [
                {
                    name: 'Graph',
                    to: {
                        name: pi.GraphStatsPageId,
                    },
                },
                {
                    name: 'Analysis',
                    to: {
                        name: pi.AnalysisStatsPageId,
                    },
                }
            ]
        },
    ]

    get homeTo() : any {
        return {
            name: pi.DashboardPageId,
        }
    }
}

</script>

<style scoped>

</style>