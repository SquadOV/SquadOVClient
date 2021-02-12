<template>
    <div>
        <v-tabs grow>
            <v-tab
                :to="allTo"
            >
                All Matches
            </v-tab>

            <v-tab
                :to="constructedTo"
            >
                Constructed
            </v-tab>

            <v-tab
                :to="arenaTo"
            >
                Arena
            </v-tab>

            <v-tab
                :to="brawlTo"
            >
                Brawl
            </v-tab>

            <v-tab
                :to="battlegroundsTo"
            >
                Battlegrounds
            </v-tab>

            <v-tab
                :to="duelsTo"
            >
                Duels
            </v-tab>
        </v-tabs>
        <router-view></router-view>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import * as pi from '@client/js/pages'

@Component
export default class HearthstoneGameLog extends Vue {
    get allTo() : any {
        return {
            name: pi.HearthstoneLogPageId,
            params: this.$route.params,
            query: this.$route.query
        }
    }

    get constructedTo() : any {
        return {
            name: pi.HearthstoneConstructedLogPageId,
            params: this.$route.params,
            query: this.$route.query
        }
    }

    get arenaTo() : any {
        return {
            name: pi.HearthstoneArenaLogPageId,
            params: this.$route.params,
            query: this.$route.query
        }
    }

    get brawlTo() : any {
        return {
            name: pi.HearthstoneBrawlLogPageId,
            params: this.$route.params,
            query: this.$route.query
        }
    }

    get battlegroundsTo() : any {
        return {
            name: pi.HearthstoneBattlegroundsLogPageId,
            params: this.$route.params,
            query: this.$route.query
        }
    }

    get duelsTo() : any {
        return {
            name: pi.HearthstoneDuelLogPageId,
            params: this.$route.params,
            query: this.$route.query
        }
    }
}

</script>

<style scoped>

</style>