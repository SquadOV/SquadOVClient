<template>
    <div>
        <router-link :to="to">
            <v-sheet
                class="task-summary"
                rounded
            >
                <div class="d-flex align-center justify-space-between task-div">
                    <v-img
                        class="mx-4"
                        :src="content.getTaskIcon(task.taskName)"
                        max-width="100"
                        max-height="100"
                        contain
                    >
                    </v-img>

                    <span class="mr-4 text-h5">{{ timeStr }}</span>

                    <div class="d-flex flex-column mr-4">
                        <span class="text-h5">
                            {{ content.getTaskName(task.taskName) }}
                        </span>

                        <span class="text-subtitle">
                            {{ content.getTaskMode(task.taskName, task.mode) }}
                        </span>
                    </div>

                    <v-spacer></v-spacer>

                    <span class="mx-4 text-h5">{{ task.score }}</span>
                </div>

                <div class="vod-div" v-if="hasVod">
                    <v-icon color="black">
                        mdi-video
                    </v-icon>
                </div>
            </v-sheet>
        </router-link>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { AimlabTaskData } from '@client/js/aimlab/aimlab_task'
import { AimlabContent, getAimlabContent } from '@client/js/aimlab/aimlab_content'
import { standardFormatTime } from '@client/js/time'
import { VodAssociation } from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import * as pi from '@client/js/pages'

@Component
export default class AimlabTaskSummaryDisplay extends Vue {
    @Prop({ required : true })
    task! : AimlabTaskData

    @Prop()
    syncVod! : VodAssociation | null
    vod: VodAssociation | null = null

    content : AimlabContent = getAimlabContent()

    get hasVod() : boolean {
        return !!this.vod
    }

    get to() : any {
        return {
            name: pi.AimlabMatchPageId,
            params: {
                taskId: this.task.matchUuid
            }
        }
    }

    get timeStr() : string {
        return standardFormatTime(this.task.createDate)
    }

    @Watch('task', {deep: true})
    refreshVod() {
        apiClient.findVodFromMatchUserUuid(this.task.matchUuid, this.$store.state.currentUser!.uuid).then((resp : ApiData<VodAssociation>) => {
            this.vod = resp.data
        }).catch((err : any) => {
            this.vod = null
        }).finally(() => {
            this.$emit('update:syncVod', this.vod)
        })
    }

    mounted () {
        this.refreshVod()
    }
}

</script>

<style scoped>

.task-summary {
    width: 100%;
    position: relative;
}

.task-div {
    min-height: 125px;
}


</style>