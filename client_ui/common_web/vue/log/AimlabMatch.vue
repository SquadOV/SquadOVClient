<template>
    <loading-container :is-loading="!data">
        <template v-slot:default="{ loading }">
            <div v-if="!loading">
                <div class="d-flex align-center">
                    <aimlab-task-summary-display
                        class="mb-4 flex-grow-1"
                        :task="data"
                        :sync-vod.sync="vod"
                        :user-id="userId"
                    >
                    </aimlab-task-summary-display>

                    <match-favorite-button
                        :match-uuid="taskId"
                    >
                    </match-favorite-button>

                    <match-share-button
                        :match-uuid="taskId"
                        :game="SquadOvGames.AimLab"
                        :graphql-stats="statPermissions"
                        :permissions="matchPermissions"
                        :full-path="$route.fullPath"
                        :timestamp="timestamp"
                        :user-id="userId"
                        :is-local="!!vod ? vod.isLocal : false"
                    >
                    </match-share-button>
                </div>

                <v-row no-gutters>
                    <v-col :cols="theaterMode ? 12 : 8">
                        <template v-if="!!vod">
                            <video-player
                                ref="player"
                                class="mb-1"
                                :vod="vod"
                                id="task-vod"
                                :current-time.sync="vodTime"
                                :enable-draw="enableDraw"
                                :current-ts.sync="timestamp"
                                :match-uuid="taskId"
                                :game="SquadOvGames.AimLab"
                                :graphql-stats="statPermissions"
                                :permissions="matchPermissions"
                                :full-path="$route.fullPath"
                                :timestamp="timestamp"
                                :user-id="userId"
                                :is-local="!!vod ? vod.isLocal : false"
                                @toggle-theater-mode="theaterMode = !theaterMode"
                                :player-height.sync="currentPlayerHeight"
                            >
                            </video-player>

                            <aimlab-vod-picker
                                v-if="!!data"
                                class="mb-2"
                                :task-id="taskId"
                                :task-name="data.taskName"
                                :vod="vod"
                                :timestamp="vodTime"
                                disable-favorite
                                :enable-draw.sync="enableDraw"
                            >
                            </aimlab-vod-picker>
                        </template>
                    </v-col>

                    <v-col v-if="!theaterMode" cols="4">
                        <generic-match-sidebar
                            v-if="!!vod"
                            :match-uuid="taskId"
                            :video-uuid="vod.videoUuid"
                            hide-events
                            :style="eventsStyle"
                            :current-tm="vodTime"
                            :start-tm="vod.startTime"
                            @go-to-time="goToVodTime(arguments[0])"
                        >
                        </generic-match-sidebar>
                    </v-col>
                </v-row>

                <aimlab-task-performance-history
                    :task="data"
                    :user-id="userId"
                    :stats.sync="statPermissions"
                >
                </aimlab-task-performance-history>
            </div>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Component, { mixins } from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { AimlabTaskData } from '@client/js/aimlab/aimlab_task'
import { apiClient, ApiData } from '@client/js/api'
import { cleanVodAssocationData } from '@client/js/squadov/vod'
import { SquadOvGames } from '@client/js/squadov/game'
import { StatPermission } from '@client/js/stats/statPrimitives'

import AimlabTaskSummaryDisplay from '@client/vue/utility/aimlab/AimlabTaskSummaryDisplay.vue'
import AimlabTaskPerformanceHistory from '@client/vue/utility/aimlab/AimlabTaskPerformanceHistory.vue'
import AimlabVodPicker from '@client/vue/utility/aimlab/AimlabVodPicker.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'
import MatchShareButton from '@client/vue/utility/squadov/MatchShareButton.vue'
import MatchFavoriteButton from '@client/vue/utility/squadov/MatchFavoriteButton.vue'
import MatchShareBase from '@client/vue/log/MatchShareBase'
import CommonComponent from '@client/vue/CommonComponent'
import GenericMatchSidebar from '@client/vue/utility/GenericMatchSidebar.vue'

@Component({
    components: {
        AimlabTaskSummaryDisplay,
        AimlabTaskPerformanceHistory,
        AimlabVodPicker,
        LoadingContainer,
        VideoPlayer,
        MatchShareButton,
        MatchFavoriteButton,
        GenericMatchSidebar,
    }
})
export default class AimlabMatch extends mixins(CommonComponent, MatchShareBase) {
    SquadOvGames: any = SquadOvGames

    @Prop({required: true})
    taskId! : string

    @Prop({required: true})
    userId!: number

    data : AimlabTaskData | null = null
    vodTime: Date | null = null
    enableDraw: boolean = false
    statPermissions: StatPermission[] = []
    theaterMode: boolean = false
    currentPlayerHeight : number = 0

    $refs!: {
        player: VideoPlayer
    }

    @Watch('taskId')
    refreshTask() {
        this.refreshMatchPermissions(this.taskId, SquadOvGames.AimLab)
        apiClient.accessToken().getAimlabTaskData(this.taskId, this.userId).then((resp : ApiData<AimlabTaskData>) => {
            this.data = resp.data
        }).catch((err : any) => {
            console.error('Failed to get Aim Lab task data: ' + err)
        })
    }

    mounted() {
        this.refreshTask()
    }

    get eventsStyle() : any {
        return {
            'height': `${this.currentPlayerHeight + this.vodPickerHeight}px`,
        }
    }

    goToVodTime(tm : Date) {
        if (!this.vod) {
            return
        }
        
        let diffMs = tm.getTime() - this.vod.startTime.getTime()
        this.$refs.player.goToTimeMs(diffMs, false)
    }
}

</script>

<style scoped>

#task-vod {
    width: 100%;
}

</style>