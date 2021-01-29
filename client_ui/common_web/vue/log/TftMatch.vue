<template>
    <loading-container :is-loading="!currentMatch">
        <template v-slot:default="{ loading }">
            <v-container fluid v-if="!loading">
                <v-row>
                    <v-col cols="12">
                        <tft-match-summary
                            class="full-width"
                            :match="playerSummary"
                            :puuid="puuid"
                            :user-id="userId"
                        >
                        </tft-match-summary>
                    </v-col>
                </v-row>

                <v-row>
                    <video-player
                        class="mb-4"
                        :vod="vod"
                        id="task-vod"
                        disable-theater
                    >
                    </video-player>

                    <tft-vod-picker
                        :match-uuid="matchUuid"
                        :puuid="puuid"
                        :vod.sync="vod"
                        :match="currentMatch"
                    >
                    </tft-vod-picker>
                </v-row>

                <v-row>
                    <v-col cols="12">
                        <tft-scoreboard
                            :puuid="puuid"
                            :match="currentMatch"
                            :patch="patch"
                        >
                        </tft-scoreboard>
                    </v-col>
                </v-row>
            </v-container>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { WrappedTftMatch, TftPlayerMatchSummary, TftParticipant, getTftSetNumber } from '@client/js/tft/matches'
import { VodAssociation } from '@client/js/squadov/vod'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'
import TftMatchSummary from '@client/vue/utility/tft/TftMatchSummary.vue'
import TftScoreboard from '@client/vue/utility/tft/TftScoreboard.vue'
import TftVodPicker from '@client/vue/utility/tft/TftVodPicker.vue'

@Component({
    components: {
        LoadingContainer,
        VideoPlayer,
        TftMatchSummary,
        TftScoreboard,
        TftVodPicker,
    }
})
export default class TftMatch extends Vue {
    @Prop({required: true})
    puuid!: string

    @Prop({required: true})
    matchUuid!: string

    @Prop({required: true})
    userId!: number

    currentMatch: WrappedTftMatch | null = null
    vod: VodAssociation | null = null

    get participantMap(): Map<string, TftParticipant> {
        let ret = new Map()
        if (!!this.currentMatch) {
            for (let p of this.currentMatch.data.info.participants) {
                ret.set(p.puuid, p)
            }
        }
        return ret
    }

    get patch(): string {
        if (!this.currentMatch) {
            return ''
        }
        return getTftSetNumber(this.currentMatch.data.info.tftSetNumber, this.currentMatch.data.info.gameVersion)
    }

    get playerSummary(): TftPlayerMatchSummary | null {
        if (!this.currentMatch) {
            return null
        }

        let participant = this.participantMap.get(this.puuid)
        if (!participant) {
            return null
        }

        return {
            matchUuid: this.matchUuid,
            gameDatetime: this.currentMatch.data.info.gameDatetime,
            gameLength: this.currentMatch.data.info.gameLength,
            gameVariation: this.currentMatch.data.info.gameVariation,
            gameVersion: this.currentMatch.data.info.gameVersion,
            queueId: this.currentMatch.data.info.queueId,
            tftSetNumber: this.currentMatch.data.info.tftSetNumber,
            companion: participant.companion,
            level: participant.level,
            placement: participant.placement,
            traits: participant.traits,
            units: participant.units,
            hasVod: !!this.vod,
            puuid: this.puuid,
        }
    }

    @Watch('matchUuid')
    refreshData() {
        apiClient.getTftMatch(this.matchUuid).then((resp: ApiData<WrappedTftMatch>) => {
            this.currentMatch = resp.data
        }).catch((err: any) => {
            console.log('Failed to get TFT match: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>

<style scoped>

#task-vod {
    width: 100%;
}

</style>