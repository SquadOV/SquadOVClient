<template>
    <loading-container :is-loading="!data">
        <template v-slot:default="{ loading }">
            <div v-if="!loading">
                <aimlab-task-summary-display
                    class="mb-4"
                    :task="data"
                    :sync-vod.sync="vod"
                >
                </aimlab-task-summary-display>

                <video-player
                    class="mb-4"
                    v-if="!!vod"
                    id="task-vod"
                    disable-theater
                >
                </video-player>

                <aimlab-task-performance-history
                    :task="data"
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

import AimlabTaskSummaryDisplay from '@client/vue/utility/aimlab/AimlabTaskSummaryDisplay.vue'
import AimlabTaskPerformanceHistory from '@client/vue/utility/aimlab/AimlabTaskPerformanceHistory.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'

@Component({
    components: {
        AimlabTaskSummaryDisplay,
        AimlabTaskPerformanceHistory,
        LoadingContainer,
        VideoPlayer
    }
})
export default class AimlabMatch extends Vue {
    @Prop({required: true})
    taskId! : string
    vod : VodAssociation | null = null

    data : AimlabTaskData | null = null

    @Watch('taskId')
    refreshTask() {
        apiClient.getAimlabTaskData(this.taskId).then((resp : ApiData<AimlabTaskData>) => {
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