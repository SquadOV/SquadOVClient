<template>
    <v-navigation-drawer app fixed clipped permanent>
        <v-list>
            <v-list-item>
                <v-list-item-avatar>
                    <v-img width="64px" contain src="/assets/icon.png"></v-img>
                </v-list-item-avatar>

                <v-list-item-content>
                    <v-list-item-title>
                        Admin Dashboard
                    </v-list-item-title>
                </v-list-item-content>
            </v-list-item>
        </v-list>

        <v-divider></v-divider>

        <v-list dense expand>
            <nav-bar-item
                v-for="(item, idx) in navItems"
                :key="idx"
                :item="item"
            >
            </nav-bar-item>
        </v-list>
    </v-navigation-drawer>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import NavBarItem from '@client/vue/NavBarItem.vue'

@Component({
    components: {
        NavBarItem
    }
})
export default class AppNav extends Vue {
    navItems: any[] = [
        {
            text: 'KPI Dashboard',
            to: '/'
        }
        {
            text: 'Metrics',
            open: true,
            children: [
                {
                    text: 'Active Users',
                    to: '/metrics/active-users',
                },
                {
                    text: 'Average Age',
                    to: '/metrics/average-age',
                },
                {
                    text: 'Lost Users',
                    to: '/metrics/lost-users',
                },
                {
                    text: 'Installed Churn',
                    to: '/metrics/churn',
                },
                {
                    text: 'Registrations',
                    to: '/metrics/registrations',
                },
                {
                    text: 'Squads',
                    to: '/metrics/squads',
                },
                {
                    text: 'Squad Invites',
                    to: '/metrics/squadinvs',
                },
                {
                    text: 'Games',
                    to: '/metrics/games',
                },
                {
                    text: 'VODs',
                    to: '/metrics/vods',
                },
                {
                    text: 'Clips',
                    to: '/metrics/clips',
                },
            ]
        },
        {
            text: 'Cohorts',
            open: true,
            children: [
                {
                    text: 'Installed Users',
                    to: '/cohorts/installed-users'
                },
                {
                    text: 'Active Users',
                    to: '/cohorts/active-users'
                },
                {
                    text: 'Recording Users',
                    to: '/cohorts/recording-users'
                },
            ]
        },
        {
            text: 'Referrals',
            open: true,
            children: [
                {
                    text: 'Campaigns',
                    to: '/referrals/campaigns'
                },
                {
                    text: 'Users',
                    to: '/referrals/users'
                },
            ]
        }
    ]
}

</script>