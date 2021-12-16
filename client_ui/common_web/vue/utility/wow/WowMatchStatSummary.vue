<template>
    <div class="ma-4">
        <loading-container :is-loading="!stats">
            <template v-slot:default="{ loading }">
                <v-container fluid v-if="!loading">
                    <v-row>
                        <v-col cols="4">
                            <wow-stat-item-list-table
                                :data="stats.damageDealt"
                                :match-characters="matchCharacters"
                                :friendly-team="friendlyTeam"
                                :patch="patch"
                                label="Damage Dealt"
                            >
                            </wow-stat-item-list-table>
                        </v-col>

                        <v-col cols="4">
                            <wow-stat-item-list-table
                                :data="stats.heals"
                                :match-characters="matchCharacters"
                                :friendly-team="friendlyTeam"
                                :patch="patch"
                                label="Heals"
                            >
                            </wow-stat-item-list-table>
                        </v-col>

                        <v-col cols="4">
                            <wow-stat-item-list-table
                                :data="stats.damageReceived"
                                :match-characters="matchCharacters"
                                :friendly-team="friendlyTeam"
                                :patch="patch"
                                label="Damage Received"
                            >
                            </wow-stat-item-list-table>
                        </v-col>
                    </v-row>
                </v-container>
            </template>
        </loading-container>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { WowCharacter } from '@client/js/wow/character'
import { WowMatchStatSummaryData } from '@client/js/wow/stats'
import { apiClient, ApiData } from '@client/js/api'
import WowStatItemListTable from '@client/vue/utility/wow/WowStatItemListTable.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'

@Component({
    components: {
        WowStatItemListTable,
        LoadingContainer,
    }
})
export default class WowMatchStatSummary extends Vue {
    @Prop({required: true})
    matchUuid!: string

    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    matchCharacters!: WowCharacter[]

    @Prop({default: 0})
    friendlyTeam!: number

    @Prop({required: true})
    patch!: string

    stats: WowMatchStatSummaryData | null = null

    @Watch('matchUuid')
    @Watch('userId')
    refreshData() {
        this.stats = null

        apiClient.getWowStatSummary(this.userId, this.matchUuid).then((resp: ApiData<WowMatchStatSummaryData>) => {
            this.stats = resp.data
        }).catch((err: any) => {
            console.warn('Failed to get wow stat summary: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>