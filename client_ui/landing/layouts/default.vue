<template>
    <v-app>
        <v-app-bar
            app
            id="nav-bar"
            dense
        >
            <v-toolbar-title class="mr-4">
                <NuxtLink to="/">
                    <div class="d-flex align-center">
                        <v-img
                            class="mr-4"
                            src="/assets/icon.png"
                            contain
                            width="32px"
                            height="32px"
                        >
                        </v-img>

                        <span>SquadOV</span>
                    </div>
                </NuxtLink>
            </v-toolbar-title>

            <v-menu bottom offset-y>
                <template v-slot:activator="{on, attrs}">
                    <v-btn text v-bind="attrs" v-on="on">
                        Games
                        <v-icon small>
                            mdi-chevron-down
                        </v-icon> 
                    </v-btn>
                </template>

                <v-list dense class="column-menu-list">
                    <template v-for="(item, idx) in gameNav">
                        <v-list-item class="column-item" :to="item.link" :key="idx">
                            <v-list-item-avatar rounded="0" width="24" height="24">
                                <v-img
                                    :src="item.icon"
                                    contain
                                >
                                </v-img>
                            </v-list-item-avatar>
                            <v-list-item-title>{{ item.name }}</v-list-item-title>
                        </v-list-item>
                    </template>
                </v-list>
            </v-menu>

<!--
            <v-menu bottom offset-y>
                <template v-slot:activator="{on, attrs}">
                    <v-btn text v-bind="attrs" v-on="on">
                        Solutions
                        <v-icon small>
                            mdi-chevron-down
                        </v-icon> 
                    </v-btn>
                </template>

                <v-list dense>
                    <v-list-item to="/coach/">
                        <v-list-item-icon>
                            <v-icon>
                                mdi-school
                            </v-icon>
                        </v-list-item-icon>
                        <v-list-item-title>Coaches</v-list-item-title>
                    </v-list-item>

                    <v-list-item to="/content/">
                        <v-list-item-icon>
                            <v-icon>
                                mdi-twitch
                            </v-icon>
                        </v-list-item-icon>
                        <v-list-item-title>Content Creators</v-list-item-title>
                    </v-list-item>

                    <v-list-item to="/esports/">
                        <v-list-item-icon>
                            <v-icon>
                                mdi-trophy
                            </v-icon>
                        </v-list-item-icon>
                        <v-list-item-title>Esports Teams</v-list-item-title>
                    </v-list-item>

                    <v-list-item to="/production/">
                        <v-list-item-icon>
                            <v-icon>
                                mdi-video
                            </v-icon>
                        </v-list-item-icon>
                        <v-list-item-title>Production</v-list-item-title>
                    </v-list-item>
                </v-list>
            </v-menu>
-->
            <v-btn href="https://support.squadov.gg" text>
                Support
            </v-btn>

            <v-btn href="https://blog.squadov.gg" text>
                Blog
            </v-btn>

            <v-spacer></v-spacer>

            <v-btn
                text
                href="https://discord.gg/jVyfv9ZHHV"
                target="_blank"
            >
                <v-icon class="mr-2">
                    mdi-discord
                </v-icon>

                Join our Discord!
            </v-btn>

            <v-btn
                class="ml-2"
                color="info"
                href="https://app.squadov.gg"
            >
                Web Client
            </v-btn>
        </v-app-bar>
        
        <v-main>
            <v-container fluid class="pa-0">
                <Nuxt />
            </v-container>
        </v-main>

        <v-sheet
            id="cookie-banner"
            sticky
            v-if="showHideCookieBanner"
        >
            <v-container fluid>
                <v-row justify="center">
                    <v-col cols="8">
                        <div class="d-flex align-center">
                            <div>
                                <div class="text-h5">
                                    Just another cookie banner.
                                </div>

                                <div class="text-subtitle-2">
                                    We use cookies to help us better reach out to gamers just like you!
                                    You can read more in our <a href="/cookie">Cookie Policy</a> and <a href="/privacy">Privacy Policy</a>.
                                </div>
                            </div>

                            <v-spacer></v-spacer>

                            <v-btn class="mr-4" color="secondary" @click="decline">
                                No, thanks
                            </v-btn>

                            <v-btn color="success" @click="accept">
                                Accept All Cookies
                            </v-btn>
                        </div>
                    </v-col>
                </v-row>
            </v-container>
        </v-sheet>

        <script charset="utf-8" type="text/javascript" src="https://js.hsforms.net/forms/shell.js"></script>
    </v-app>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'
import { initGtm } from '~/plugins/gtm'

@Component
export default class DefaultLayout extends Vue {
    showHideCookieBanner: boolean = false

    get cookieOpts() : any {
        return {
            'path': '/',
            'maxAge': 60 * 60 * 24 * 365,
            'domain': 'squadov.gg',
        }
    }

    accept() {
        initGtm()

        //@ts-ignore
        this.$cookies.set('squadov-cookie-consent', 'yes', this.cookieOpts)
        this.showHideCookieBanner = false
    }
    
    decline() {
        //@ts-ignore
        this.$cookies.set('squadov-cookie-consent', 'no', this.cookieOpts)
        this.showHideCookieBanner = false
    }

    mounted() {
        //@ts-ignore
        let consent = this.$cookies.get('squadov-cookie-consent')
        if (consent === 'yes') {
            this.accept()
        } else if (!consent) {
            this.showHideCookieBanner = true
        }
    }


    get gameNav(): any[] {
        return [
            {
                name: 'Aim Lab',
                link: '/aimlab/',
                icon: '/assets/icons/aimlab-logo.png',
            },
            {
                name: 'CS:GO',
                link: '/csgo/',
                icon: '/assets/icons/csgo-logo.png',
            },
            {
                name: 'Dota 2',
                link: '/dota2/',
                icon: '/assets/icons/dota2-logo.png',
            },
            {
                name: 'Fortnite',
                link: '/fortnite/',
                icon: '/assets/icons/fortnite-logo.png',
            },
            {
                name: 'Hearthstone',
                link: '/hearthstone/',
                icon: '/assets/icons/hearthstone-logo.png',
            },
            {
                name: 'League of Legends',
                link: '/lol/',
                icon: '/assets/icons/lol-logo.png',
            },
            {
                name: 'Rocket League',
                link: '/rocketleague/',
                icon: '/assets/icons/rocketleague-logo.png',
            },
            {
                name: 'Teamfight Tactics',
                link: '/tft/',
                icon: '/assets/icons/tft-logo.png',
            },
            {
                name: 'Valorant',
                link: '/valorant/',
                icon: '/assets/icons/valorant-logo.png',
            },
            {
                name: 'World of Warcraft',
                link: '/wow/',
                icon: '/assets/icons/wow-logo.png',
            },
        ]
    }
}

</script>

<style scoped>

#cookie-banner {
    position: sticky;
    bottom: 0px;
    z-index: 3;
}

#nav-bar a {
    color: inherit;
    text-decoration: none;
}

.column-menu-list {
    display: flex;
    flex-wrap: wrap;
    flex-direction: row;
    width: 400px;
}

.column-item {
    flex-grow: 0;
    flex-basis: 50%;
}

</style>
