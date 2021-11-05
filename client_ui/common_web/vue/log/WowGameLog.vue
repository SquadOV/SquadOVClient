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

            <v-tab
                :to="instanceTo"
            >
                Instances
            </v-tab>
        </v-tabs>
        <router-view></router-view>
    </div>
</template>

<script lang="ts">

import Component, { mixins } from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import * as pi from '@client/js/pages'
import CommonComponent from '../CommonComponent'

@Component
export default class WoWGameLog extends mixins(CommonComponent) {
    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    guid!: string

    get encounterTo(): any {
        return {
            name: pi.WowEncounterLogPageId,
            params: this.$route.params,
            query: this.cleanQuery
        }
    }

    get keystoneTo(): any {
        return {
            name: pi.WowChallengeLogPageId,
            params: this.$route.params,
            query: this.cleanQuery
        }
    }

    get arenaTo(): any {
        return {
            name: pi.WowArenaLogPageId,
            params: this.$route.params,
            query: this.cleanQuery
        }
    }

    get instanceTo(): any {
        return {
            name: pi.WowInstanceLogPageId,
            params: this.$route.params,
            query: this.cleanQuery
        }
    }
}

</script>

<style scoped>

</style>