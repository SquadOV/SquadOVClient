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
                        :user-id="userId"
                    >
                    </match-share-button>
                </div>

                <template v-if="!!vod">
                    <video-player
                        class="mb-1"
                        :vod="vod"
                        id="task-vod"
                        disable-theater
                        :current-time.sync="vodTime"
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
                    >
                    </aimlab-vod-picker>
                </template>

                <aimlab-task-performance-history
                    :task="data"
                    :user-id="userId"
                >
                </aimlab-task-performance-history>
            </div>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { AimlabTaskData } from '@client/js/aimlab/aimlab_task'
import { apiClient, ApiData } from '@client/js/api'
import { VodAssociation } from '@client/js/squadov/vod'
import { SquadOvGames } from '@client/js/squadov/game'

import AimlabTaskSummaryDisplay from '@client/vue/utility/aimlab/AimlabTaskSummaryDisplay.vue'
import AimlabTaskPerformanceHistory from '@client/vue/utility/aimlab/AimlabTaskPerformanceHistory.vue'
import AimlabVodPicker from '@client/vue/utility/aimlab/AimlabVodPicker.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'
import MatchShareButton from '@client/vue/utility/squadov/MatchShareButton.vue'
import MatchFavoriteButton from '@client/vue/utility/squadov/MatchFavoriteButton.vue'

@Component({
    components: {
        AimlabTaskSummaryDisplay,
        AimlabTaskPerformanceHistory,
        AimlabVodPicker,
        LoadingContainer,
        VideoPlayer,
        MatchShareButton,
        MatchFavoriteButton
    }
})
export default class AimlabMatch extends Vue {
    SquadOvGames: any = SquadOvGames

    @Prop({required: true})
    taskId! : string

    @Prop({required: true})
    userId!: number

    // Synced from the task summary display. Kind of a yikes there...
    vod : VodAssociation | null = null
    data : AimlabTaskData | null = null
    vodTime: Date | null = null

    @Watch('taskId')
    refreshTask() {
        apiClient.getAimlabTaskData(this.taskId, this.userId).then((resp : ApiData<AimlabTaskData>) => {
            this.data = resp.data
        }).catch((err : any) => {
            console.log('Failed to get Aim Lab task data: ' + err)
        })
    }

    mounted() {
        this.refreshTask()
    }
}

</script>

<style scoped>

#task-vod {
    width: 100%;
}

</style>