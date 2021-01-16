<template>
    <loading-container :is-loading="!currentMatch">
        <template v-slot:default="{ loading }">
            <v-container fluid v-if="!loading">
                <v-row>
                    <tft-match-summary
                        class="m-4"
                        :match="playerSummary"
                        :puuid="puuid"
                        :user-id="userId"
                    >
                    </tft-match-summary>
                </v-row>

                <video-player
                    class="mb-4"
                    v-if="!!vod"
                    :vod="vod"
                    id="task-vod"
                    disable-theater
                >
                </video-player>
            </div>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { WrappedTftMatch, TftPlayerMatchSummary, TftParticipant } from '@client/js/tft/matches'
import { VodAssociation } from '@client/js/squadov/vod'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'
import TftMatchSummary from '@client/vue/utility/tft/TftMatchSummary.vue'

@Component({
    components: {
        LoadingContainer,
        VideoPlayer,
        TftMatchSummary
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
            hasVod: !!this.vod
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