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
        }
    }

    get constructedTo() : any {
        return {
            name: pi.HearthstoneConstructedLogPageId,
        }
    }

    get arenaTo() : any {
        return {
            name: pi.HearthstoneArenaLogPageId,
        }
    }

    get brawlTo() : any {
        return {
            name: pi.HearthstoneBrawlLogPageId,
        }
    }

    get battlegroundsTo() : any {
        return {
            name: pi.HearthstoneBattlegroundsLogPageId,
        }
    }

    get duelsTo() : any {
        return {
            name: pi.HearthstoneDuelLogPageId,
        }
    }
}

</script>

<style scoped>

</style>