<template>
    <div>
        <router-link :to="to">
            <v-sheet
                class="task-summary"
                rounded
            >
                <v-row no-gutters v-if="!!currentArenaRun">
                </v-row>
                <v-row justify="center" v-else>
                    <v-progress-circular indeterminate size=64></v-progress-circular>
                </v-row>
            </v-sheet>
        </router-link>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { HearthstoneArenaRun } from '@client/js/hearthstone/hearthstone_arena'
import { apiClient, ApiData } from '@client/js/api'

@Component
export default class HearthstoneArenaSummaryDisplay extends Vue {
    @Prop({required: true})
    arenaUuid!: string

    currentArenaRun: HearthstoneArenaRun | null = null

    get to() : any {
        return {
        }
    }

    refreshData() {
        apiClient.getHearthstoneArenaRun(this.arenaUuid, this.$store.state.currentUser!.id).then((resp : ApiData<HearthstoneArenaRun>) => {
            this.currentArenaRun = resp.data
        }).catch((err: any) => {
            console.log('Failed to get Hearthstone Arena Run: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>

<style scoped>

.task-summary {
    width: 100%;
    position: relative;
}

</style>