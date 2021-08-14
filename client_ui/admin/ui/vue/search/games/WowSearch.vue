<template>
    <div class="full-width">
        <div class="d-flex align-center ma-4">
            <v-text-field
                v-model="matchSearch"
                label="Match ID"
                solo
                dense
                hide-details
            >
            </v-text-field>

            <v-text-field
                class="ml-4"
                v-model="userSearch"
                label="User Id"
                solo
                dense
                hide-details
            >
            </v-text-field>

            <v-btn
                class="ml-4"
                color="primary"
                :disabled="!canSearch"
                @click="searchMatches"
                :loading="searching"
            >
                Search
            </v-btn>
        </div>

        <div v-if="games.length > 0">
            <template v-for="(g, idx) in games">
                <v-container :key="idx" fluid class="mb-4">
                    <v-row>
                        <v-col cols="2">
                            <b>Match ID</b>
                        </v-col>

                        <v-col cols="10">
                            {{ g.matchUuid }}
                        </v-col>
                    </v-row>

                    <v-row>
                        <v-col cols="2">
                            <b>User ID</b>
                        </v-col>

                        <v-col cols="10">
                            {{ g.userUuid }}
                        </v-col>
                    </v-row>

                    <v-row>
                        <v-col cols="2">
                            <b>Video ID</b>
                        </v-col>

                        <v-col cols="10">
                            {{ g.videoUuid }}
                        </v-col>
                    </v-row>

                    <v-row>
                        <v-col cols="2">
                            <b>Time</b>
                        </v-col>

                        <v-col cols="10">
                            {{ g.time }}
                        </v-col>
                    </v-row>

                    <v-row>
                        <v-col cols="2">
                            <b>WoW Patch</b>
                        </v-col>

                        <v-col cols="10">
                            {{ g.gameVersion }}
                        </v-col>
                    </v-row>

                    <template v-if="!!g.arena">
                        <v-row>
                            <v-col cols="2">
                                <b>Arena Type</b>
                            </v-col>

                            <v-col cols="10">
                                {{ g.arena.type }}
                            </v-col>
                        </v-row>
                    </template>

                    <template v-if="!!g.challenge">
                        <v-row>
                            <v-col cols="2">
                                <b>Challenge Name</b>
                            </v-col>

                            <v-col cols="10">
                                {{ g.challenge.name }}
                            </v-col>
                        </v-row>

                        <v-row>
                            <v-col cols="2">
                                <b>Challenge Keystone</b>
                            </v-col>

                            <v-col cols="10">
                                {{ g.challenge.keystone }}
                            </v-col>
                        </v-row>

                        <v-row>
                            <v-col cols="2">
                                <b>Challenge Clear</b>
                            </v-col>

                            <v-col cols="10">
                                {{ g.challenge.success }}
                            </v-col>
                        </v-row>
                    </template>

                    <template v-if="!!g.encounter">
                        <v-row>
                            <v-col cols="2">
                                <b>Encounter Name</b>
                            </v-col>

                            <v-col cols="10">
                                {{ g.encounter.name }}
                            </v-col>
                        </v-row>

                        <v-row>
                            <v-col cols="2">
                                <b>Encounter Difficulty</b>
                            </v-col>

                            <v-col cols="10">
                                {{ g.encounter.difficulty }}
                            </v-col>
                        </v-row>

                        <v-row>
                            <v-col cols="2">
                                <b>Encounter Clear</b>
                            </v-col>

                            <v-col cols="10">
                                {{ g.encounter.success }}
                            </v-col>
                        </v-row>
                    </template>
                </v-container>
            </template>
        </div>

        <div class="text-h2" v-else>
            No Games Found
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { WowGame, searchForGames } from '@client/ts/games'

@Component
export default class WowSearch extends Vue {
    matchSearch: string = ''
    userSearch: string = ''
    searching: boolean = false
    games: WowGame[] = []

    searchMatches() {
        searchForGames<WowGame>('wow', {
            match: this.matchSearch,
            user: this.userSearch,
        }).then((resp: WowGame[]) => {
            this.games = resp
        }).catch((err: any) => {
            console.log('Failed to get wow games: ', err)
        })
    }

    get canSearch(): boolean {
        return this.matchSearch.trim() != '' || this.userSearch.trim() != ''
    }
}

</script>