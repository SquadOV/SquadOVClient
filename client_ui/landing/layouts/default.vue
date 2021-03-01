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

                        <span class="mr-1">SquadOV</span>
                        <span class="text-caption">(Beta)</span>
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

                <v-list dense>
                    <v-list-item to="/aimlab/">
                        <v-list-item-title>Aim Lab</v-list-item-title>
                    </v-list-item>
                    <v-list-item to="/hearthstone/"> 
                        <v-list-item-title>Hearthstone</v-list-item-title>
                    </v-list-item>
                    <v-list-item to="/valorant/">
                        <v-list-item-title>Valorant</v-list-item-title>
                    </v-list-item>
                    <v-list-item to="/wow/">
                        <v-list-item-title>World of Warcraft</v-list-item-title>
                    </v-list-item>
                </v-list>
            </v-menu>

            <v-btn href="https://support.squadov.gg" text>
                Support
            </v-btn>

            <v-btn href="https://blog.squadov.gg" text>
                Blog
            </v-btn>

            <v-spacer></v-spacer>

            <v-btn
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

</style>
