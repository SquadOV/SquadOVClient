<template>
    <v-app>
        <v-app-bar
            app
            id="nav-bar"
            dense
            clipped-left
            hide-on-scroll
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

                        <span>SquadOV Support</span>
                    </div>
                </NuxtLink>
            </v-toolbar-title>

            <v-spacer></v-spacer>

            <v-btn
                class="mr-2"
                color="success"
                href="https://www.squadov.gg"
            >
                Home
            </v-btn>

            <v-btn
                color="info"
                href="https://app.squadov.gg"
            >
                Web Client
            </v-btn>
        </v-app-bar>

        <v-navigation-drawer
            app
            fixed
            clipped
            permanent
            width="300"
        >
            <v-list dense expand>
                <NavBarItem
                    v-for="(item, idx) in navItems"
                    :key="idx"
                    :item="item"
                />
            </v-list>
        </v-navigation-drawer>
        
        <v-main>
            <v-container fluid class="pa-0 full-height">
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
    </v-app>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'
import { initGtm } from '~/plugins/gtm'

@Component
export default class DefaultLayout extends Vue {
    showHideCookieBanner: boolean = false
    navItems: any[] = [
        {
            text: 'Welcome',
            to: '/user-manual/welcome',
        },
        {
            text: 'Account',
            open: true,
            children: [
                {
                    text: 'Registration',
                    to: '/user-manual/account#registration',
                },
                {
                    text: 'Login',
                    to: '/user-manual/account#login',
                },
                {
                    text: 'Forgot Password',
                    to: '/user-manual/account#forgotpw',
                },
                {
                    text: 'User Profile',
                    to: '/user-manual/account#profile',
                },
            ]
        },
        {
            text: 'Dashboard',
            open: true,
            children: [
                {
                    text: 'Overview',
                    to: '/user-manual/dashboard#overview',
                },
                {
                    text: 'App Navigation',
                    to: '/user-manual/dashboard#appnav',
                },
                {
                    text: 'Squad Management',
                    to: '/user-manual/dashboard#squads',
                },
                {
                    text: 'Recent Matches',
                    to: '/user-manual/dashboard#recent',
                },
                {
                    text: 'Recorded Playtime',
                    to: '/user-manual/dashboard#playtime',
                },
                {
                    text: 'Recording Status',
                    to: '/user-manual/dashboard#recording',
                },
                {
                    text: 'Recent Updates',
                    to: '/user-manual/dashboard#news',
                },
                {
                    text: 'Status Updates',
                    to: '/user-manual/dashboard#status',
                },
                {
                    text: 'Bug Reports',
                    to: '/user-manual/dashboard#bugs',
                },
            ]
        },
        {
            text: 'Games',
            open: true,
            children: [
                {
                    text: 'Common',
                    open: true,
                    children: [
                        {
                            text: 'Game Log',
                            to: '/user-manual/games/common#log',
                        },
                        {
                            text: 'Match Sharing',
                            to: '/user-manual/games/common#share',
                        }
                    ]
                },
                {
                    text: 'Aim Lab',
                    open: true,
                    children: [
                        {
                            text: 'Match History',
                            to: '/user-manual/games/aimlab#history',
                        },
                        {
                            text: 'Task Details',
                            to: '/user-manual/games/aimlab#details',
                        }
                    ]
                },
                {
                    text: 'Hearthstone',
                    open: true,
                    children: [
                        {
                            text: 'Log Setup',
                            to: '/user-manual/games/hearthstone#logs',
                        },
                        {
                            text: 'Match History',
                            open: true,
                            children: [
                                {
                                    text: 'Overview',
                                    to: '/user-manual/games/hearthstone#history-overview'
                                },
                                {
                                    text: 'Constructed',
                                    to: '/user-manual/games/hearthstone#constructed'
                                },
                                {
                                    text: 'Arena',
                                    to: '/user-manual/games/hearthstone#arena'
                                },
                                {
                                    text: 'Brawl',
                                    to: '/user-manual/games/hearthstone#brawl'
                                },
                                {
                                    text: 'Battlegrounds',
                                    to: '/user-manual/games/hearthstone#battlegrounds'
                                },
                                {
                                    text: 'Duels',
                                    to: '/user-manual/games/hearthstone#duels'
                                }
                            ]
                        },
                        {
                            text: 'Match Details',
                            open: true,
                            children: [
                                {
                                    text: '1v1 Matches',
                                    to: '/user-manual/games/hearthstone#details-solo'
                                },
                                {
                                    text: 'Battlegrounds',
                                    to: '/user-manual/games/hearthstone#details-bg'
                                },
                            ]
                        }
                    ]
                },
                {
                    text: 'League of Legends',
                    open: true,
                    children: [
                        {
                            text: 'Account Setup',
                            to: '/user-manual/games/lol#account',
                        },
                        {
                            text: 'Match History',
                            to: '/user-manual/games/lol#history',
                        },
                        {
                            text: 'Match Details',
                            to: '/user-manual/games/lol#details',
                        }
                    ]
                },
                {
                    text: 'Teamfight Tactics',
                    open: true,
                    children: [
                        {
                            text: 'Account Setup',
                            to: '/user-manual/games/tft#account',
                        },
                        {
                            text: 'Match History',
                            to: '/user-manual/games/tft#history',
                        },
                        {
                            text: 'Match Details',
                            to: '/user-manual/games/tft#details',
                        }
                    ]
                },
                {
                    text: 'Valorant',
                    open: true,
                    children: [
                        {
                            text: 'Account Setup',
                            to: '/user-manual/games/valorant#account',
                        },
                        {
                            text: 'Match History',
                            to: '/user-manual/games/valorant#history',
                        },
                        {
                            text: 'Match Details',
                            to: '/user-manual/games/valorant#details',
                        }
                    ]
                },
                {
                    text: 'World of Warcraft',
                    open: true,
                    children: [
                        {
                            text: 'Combat Log Setup',
                            to: '/user-manual/games/wow#combatlog',
                        },
                        {
                            text: 'Match History',
                            open: true,
                            children: [
                                {
                                    text: 'Overview',
                                    to: '/user-manual/games/wow#history-overview',
                                },
                                {
                                    text: 'Encounters',
                                    to: '/user-manual/games/wow#history-encounters',
                                },
                                {
                                    text: 'Keystones',
                                    to: '/user-manual/games/wow#history-keystones',
                                },
                                {
                                    text: 'Arenas',
                                    to: '/user-manual/games/wow#history-arenas',
                                }
                            ]
                        },
                        {
                            text: 'Match Details',
                            to: '/user-manual/games/wow#history-details',
                        }
                    ]
                },
            ]
        },
        {
            text: 'Squads',
            open: true,
            children: [
                {
                    text: 'Creating Squads',
                    to: '/user-manual/squads#create',
                },
                {
                    text: 'Squad Invites',
                    to: '/user-manual/squads#invite',
                },
                {
                    text: 'Managing Squads',
                    to: '/user-manual/squads#manage',
                },
            ]
        },
        {
            text: 'Stat Visualization',
            open: true,
            children: [
                {
                    text: 'Overview',
                    to: '/user-manual/stat-visualization#overview',
                },
                {
                    text: 'Aim Lab Stats',
                    to: '/user-manual/stat-visualization#aimlab',
                },
            ],
        },
        {
            text: 'VODs',
            open: true,
            children: [
                {
                    text: 'Video Player',
                    to: '/user-manual/vod#player',
                },
                {
                    text: 'VOD Picker',
                    to: '/user-manual/vod#picker',
                },
                {
                    text: 'Clip Editor',
                    to: '/user-manual/vod#clipeditor',
                },
                {
                    text: 'Clip Viewer',
                    to: '/user-manual/vod#clipviewer',
                },
                {
                    text: 'Clip Library',
                    to: '/user-manual/vod#cliplibrary',
                },
            ],
        },
        {
            text: 'Settings',
            open: true,
            children: [
                {
                    text: 'Video Recording',
                    to: '/user-manual/settings#video',
                },
                {
                    text: 'Audio Recording',
                    to: '/user-manual/settings#audio',
                },
                {
                    text: 'System',
                    to: '/user-manual/settings#system',
                },
            ],
        }
    ]

    get cookieOpts() : any {
        return {
            'path': '/',
            'maxAge': 60 * 60 * 24 * 365,
            'domain': 'squadov.gg'
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
}

</script>

<style scoped>

#cookie-banner {
    position: sticky;
    bottom: 0px;
    z-index: 50;
}

#nav-bar a {
    color: inherit;
    text-decoration: none;
}

.full-height {
    height: 100%;
}

</style>