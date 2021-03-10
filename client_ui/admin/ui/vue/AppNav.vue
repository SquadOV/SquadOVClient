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
            text: 'Metrics',
            open: true,
            children: [
                {
                    text: 'Active Users',
                    to: '/metrics/active-users',
                },
                {
                    text: 'Churn',
                    to: '/metrics/churn',
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