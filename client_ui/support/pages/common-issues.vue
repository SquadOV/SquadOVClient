<template>
    <div class="ma-4">
        <div class="text-h3 text-center">
            Common Issues
        </div>

        <v-expansion-panels class="mt-4 px-16">
            <v-expansion-panel>
                <v-expansion-panel-header class="font-weight-bold">
                    Local Service Error
                </v-expansion-panel-header>

                <v-expansion-panel-content>
                    There can be multiple causes of the “The local service encountered an unexpected error, please restart SquadOV and try again” message.
                    The most common fix is to download and install the <a href="https://support.microsoft.com/en-us/topic/the-latest-supported-visual-c-downloads-2647da03-1eea-4433-9aff-95f26a218cc0" target="_blank">Visual Studio 2019 C++ <b>x64</b> Redistributable.</a>
                </v-expansion-panel-content>
            </v-expansion-panel>

            <v-expansion-panel>
                <v-expansion-panel-header class="font-weight-bold">
                    VOD has no audio
                </v-expansion-panel-header>

                <v-expansion-panel-content>
                    This is generally due to SquadOV not picking up audio from either your desktop audio or microphone input.
                    Check your audio <NuxtLink to="/user-manual/settings#audio">recording settings</NuxtLink> and ensure that the correct device is selected.
                </v-expansion-panel-content>
            </v-expansion-panel>

            <v-expansion-panel>
                <v-expansion-panel-header class="font-weight-bold">
                    VOD is Broken
                </v-expansion-panel-header>

                <v-expansion-panel-content>
                    Common errors are that the VOD’s video cuts out after a certain point or the VOD takes a long time to load.
                    This generally happens when the VOD is still processing (if you see a spinning icon in the VOD picker).
                    If after a few hours the VOD is still processing, please email us at <a href="mailto:support@grchive.com">support@grchive.com</a> with the <NuxtLink to="/user-manual/games#match-sharing">match sharing URL</NuxtLink> so we can investigate.
                </v-expansion-panel-content>
            </v-expansion-panel>

            <v-expansion-panel>
                <v-expansion-panel-header class="font-weight-bold">
                    World of Warcraft Matches are not recorded
                </v-expansion-panel-header>

                <v-expansion-panel-content>
                    Generally, this is due to SquadOV being unable to detect changes to the World of Warcraft combat log.
                    Ensure that combat logs are enabled when you start the encounter/M+/arena and that you have advanced combat logging enabled.
                    See <NuxtLink to="/user-manual/games/wow#combatlog">our guide on WoW combat log addon setup</NuxtLink>.
                </v-expansion-panel-content>
            </v-expansion-panel>

            <v-expansion-panel>
                <v-expansion-panel-header class="font-weight-bold">
                    Valorant/League of Legends/TFT matches are not recorded
                </v-expansion-panel-header>

                <v-expansion-panel-content>
                    If you are playing a game developed by Riot Games, we only record if we can verify that you own the account in question.
                    Thus, ensure that you have setup your account properly:
                    <ul>
                        <li><NuxtLink to="/user-manual/games/valorant#account">Valorant Account Setup</NuxtLink></li>
                        <li><NuxtLink to="/user-manual/games/lol#account">LoL Account Setup</NuxtLink></li>
                        <li><NuxtLink to="/user-manual/games/tft#account">TFT Account Setup</NuxtLink></li>
                    </ul>
                </v-expansion-panel-content>
            </v-expansion-panel>

            <v-expansion-panel>
                <v-expansion-panel-header class="font-weight-bold">
                    VODs are not recorded
                </v-expansion-panel-header>

                <v-expansion-panel-content>
                    <div>
                        If you are on Windows 7, note that SquadOV can not record any games in fullscreen mode so you must play in windowed mode.
                    </div>

                    <div class="mt-4">
                        If you are on Windows 8 or Windows 10, please submit a bug report.
                    </div>
                </v-expansion-panel-content>
            </v-expansion-panel>

            <v-expansion-panel>
                <v-expansion-panel-header class="font-weight-bold">
                    UI is stuck on “An error occurred while connecting to SquadOV...Trying to reconnect.”
                </v-expansion-panel-header>

                <v-expansion-panel-content>
                    Navigate to %APPDATA%\SquadOV in Explorer and delete the session.json file and then restart SquadOV.

                    <GenericImage
                        class="my-2"
                        src="/assets/common-issues/common_ui_stuck.jpg"
                        alt="SquadOV session file."
                        caption="Location of SquadOV's session file."
                    />

                    This will force you to re-login.
                    If this still doesn't work, our <a href="https://squadov.statuspage.io/">servers might be down</a>.
                </v-expansion-panel-content>
            </v-expansion-panel>
        </v-expansion-panels>
    </div>
</template>
<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class CommonIssues extends Vue {
    head() : any {
        return {
            title: 'SquadOV - Common Issues',
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
                    content: "How to fix most common issues people encounter with SquadOV."
                }
            ]
        }
    }
}

</script>