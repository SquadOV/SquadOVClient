<template>
    <div>
        <v-tabs>
            <v-tab
                :to="encounterTo"
            >
                Encounters
            </v-tab>

            <v-tab
                :to="keystoneTo"
            >
                Keystones
            </v-tab>

            <v-tab
                :to="arenaTo"
            >
                Arena
            </v-tab>
        </v-tabs>
        <router-view></router-view>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import * as pi from '@client/js/pages'

@Component
export default class WoWGameLog extends Vue {
    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    guid!: string

    get encounterTo(): any {
        return {
            name: pi.WowEncounterLogPageId,
            params: this.$route.params,
            query: this.$route.query
        }
    }

    get keystoneTo(): any {
        return {
            name: pi.WowChallengeLogPageId,
            params: this.$route.params,
            query: this.$route.query
        }
    }

    get arenaTo(): any {
        return {
            name: pi.WowArenaLogPageId,
            params: this.$route.params,
            query: this.$route.query
        }
    }
}

</script>

<style scoped>

</style>