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
import Component, { mixins } from 'vue-class-component'
import * as pi from '@client/js/pages'
import CommonComponent from '../CommonComponent'

@Component
export default class HearthstoneGameLog extends mixins(CommonComponent) {
    get allTo() : any {
        return {
            name: pi.HearthstoneLogPageId,
            params: this.$route.params,
            query: this.cleanQuery
        }
    }

    get constructedTo() : any {
        return {
            name: pi.HearthstoneConstructedLogPageId,
            params: this.$route.params,
            query: this.cleanQuery
        }
    }

    get arenaTo() : any {
        return {
            name: pi.HearthstoneArenaLogPageId,
            params: this.$route.params,
            query: this.cleanQuery
        }
    }

    get brawlTo() : any {
        return {
            name: pi.HearthstoneBrawlLogPageId,
            params: this.$route.params,
            query: this.cleanQuery
        }
    }

    get battlegroundsTo() : any {
        return {
            name: pi.HearthstoneBattlegroundsLogPageId,
            params: this.$route.params,
            query: this.cleanQuery
        }
    }

    get duelsTo() : any {
        return {
            name: pi.HearthstoneDuelLogPageId,
            params: this.$route.params,
            query: this.cleanQuery
        }
    }
}

</script>

<style scoped>

</style>