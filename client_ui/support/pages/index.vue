<template>
    <div id="main-container" class="d-flex flex-column justify-center align-center">
        <div class="d-flex justify-center text-h5 font-weight-bold">
            How can we help you?
        </div>

        <div class="d-flex flex-wrap justify-center align-center mt-2">
            <NuxtLink to="/user-manual/welcome/" class="sheet-link">
                <v-sheet
                    class="mx-2 d-flex flex-column align-center justify-center"
                    width="150"
                    height="150"
                    rounded
                >
                    <v-icon x-large>
                        mdi-book-open-page-variant
                    </v-icon>

                    <div class="font-weight-bold mt-2">
                        User Manual
                    </div>
                </v-sheet>
            </NuxtLink>

            <NuxtLink to="/common-issues/" class="sheet-link">
                <v-sheet
                    class="mx-2 d-flex flex-column align-center justify-center"
                    width="150"
                    height="150"
                    rounded
                >
                    <v-icon x-large>
                        mdi-cog
                    </v-icon>

                    <div class="font-weight-bold mt-2">
                        Common Issues
                    </div>
                </v-sheet>
            </NuxtLink>

            <a href="https://squadov.statuspage.io/" target="_blank" class="sheet-link">
                <v-sheet
                    class="mx-2 d-flex flex-column align-center justify-center"
                    width="150"
                    height="150"
                    rounded
                >
                    <v-icon x-large>
                        mdi-server
                    </v-icon>

                    <div class="font-weight-bold mt-2">
                        Service Status
                    </div>
                </v-sheet>
            </a>

            <a href="https://trello.com/b/rBgnOUU0/squadov-public-roadmap" target="_blank" class="sheet-link">
                <v-sheet
                    class="mx-2 d-flex flex-column align-center justify-center"
                    width="150"
                    height="150"
                    rounded
                >
                    <v-icon x-large>
                        mdi-map
                    </v-icon>

                    <div class="font-weight-bold mt-2">
                        Public Roadmap
                    </div>
                </v-sheet>
            </a>
        </div>

        <v-divider class="mt-8" style="width: 50%"></v-divider>

        <div class="d-flex justify-center text-h5 font-weight-bold mt-8">
            Get in touch!
        </div>

        <div class="d-flex flex-wrap justify-center align-center mt-2">
            <a href="https://discord.gg/6Rj5jCVDeC" target="_blank" alt="Discord" class="sheet-link">
                <v-sheet
                    class="mx-2 d-flex flex-column align-center justify-center"
                    width="125"
                    height="125"
                    rounded
                >
                    <v-icon x-large>
                        mdi-discord
                    </v-icon>

                    <div class="font-weight-bold mt-2">
                        Discord
                    </div>
                </v-sheet>
            </a>

            <a href="https://facebook.com/squadov/" target="_blank" alt="Facebook" class="sheet-link">
                <v-sheet
                    class="mx-2 d-flex flex-column align-center justify-center"
                    width="125"
                    height="125"
                    rounded
                >
                    <v-icon x-large>
                        mdi-facebook
                    </v-icon>

                    <div class="font-weight-bold mt-2">
                        Facebook
                    </div>
                </v-sheet>
            </a>

            <a href="https://www.instagram.com/squadovgg/" target="_blank" alt="Instagram" class="sheet-link">
                <v-sheet
                    class="mx-2 d-flex flex-column align-center justify-center"
                    width="125"
                    height="125"
                    rounded
                >
                    <v-icon x-large>
                        mdi-instagram
                    </v-icon>

                    <div class="font-weight-bold mt-2">
                        Instagram
                    </div>
                </v-sheet>
            </a>

            <a href="https://twitter.com/SquadOV" target="_blank" alt="Twitter" class="sheet-link">
                <v-sheet
                    class="mx-2 d-flex flex-column align-center justify-center"
                    width="125"
                    height="125"
                    rounded
                >
                    <v-icon x-large>
                        mdi-twitter
                    </v-icon>

                    <div class="font-weight-bold mt-2">
                        Twitter
                    </div>
                </v-sheet>
            </a>

            <a href="https://www.linkedin.com/company/squadov" target="_blank" alt="LinkedIn" class="sheet-link">
                <v-sheet
                    class="mx-2 d-flex flex-column align-center justify-center"
                    width="125"
                    height="125"
                    rounded
                >
                    <v-icon x-large>
                        mdi-linkedin
                    </v-icon>

                    <div class="font-weight-bold mt-2">
                        LinkedIn
                    </div>
                </v-sheet>
            </a>

            <a href="mailto:support@grchive.com" alt="Email" class="sheet-link">
                <v-sheet
                    class="mx-2 d-flex flex-column align-center justify-center"
                    width="125"
                    height="125"
                    rounded
                >
                    <v-icon x-large>
                        mdi-email
                    </v-icon>

                    <div class="font-weight-bold mt-2">
                        Email
                    </div>
                </v-sheet>
            </a>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class Index extends Vue {
    head() : any {
        return {
            title: 'SquadOV - Support',
            htmlAttrs: {
                lang: 'en'
            },
            meta: [
                {
                    charset: 'utf-8'
                },
                {
                    name: 'viewport',
                    content: 'width=device-width, initial-scale=1'
                },
                {
                    hid: 'description',
                    name: 'description',
                    content: "Need help getting SquadOV to record VODs? Can't figure out how to get SquadOV to display match details for your favorite game? This is the right place."
                }
            ]
        }
    }
}

</script>

<style scoped>

#main-container {
    height: 100%;
}

.sheet-link {
    text-decoration: none;
}

</style>