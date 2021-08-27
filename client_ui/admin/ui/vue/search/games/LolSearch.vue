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
                            <b>Riot ID</b>
                        </v-col>

                        <v-col cols="10">
                            {{ g.riotId }}
                        </v-col>
                    </v-row>
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
import { LolGame, searchForGames } from '@client/ts/games'

@Component
export default class LolSearch extends Vue {
    matchSearch: string = ''
    userSearch: string = ''
    searching: boolean = false
    games: LolGame[] = []

    searchMatches() {
        searchForGames<LolGame>('lol', {
            match: this.matchSearch,
            user: this.userSearch,
        }).then((resp: LolGame[]) => {
            this.games = resp
        }).catch((err: any) => {
            console.error('Failed to get lol games: ', err)
        })
    }

    get canSearch(): boolean {
        return this.matchSearch.trim() != '' || this.userSearch.trim() != ''
    }
}

</script>