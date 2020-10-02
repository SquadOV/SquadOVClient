<template>
    <v-tabs v-if="content.isTaskModeSupported(task.taskName, task.mode)">
        <v-tab>
            Score
        </v-tab>

        <v-tab-item>
            <stat-container
                class="pa-4"
                :stats="[content.getTaskCommonScoreStat(task.taskName, task.mode)]"
                :title="getStatTitle(content.getTaskCommonScoreStat(task.taskName, task.mode))"
            >
                <template v-slot:default="{ data }">
                    <line-graph
                        :series-data="data"
                    >
                    </line-graph>
                </template>
            </stat-container>
        </v-tab-item>

        <template v-if="hasExtraStats">
            <v-tab>
                Task Specific
            </v-tab>

            <v-tab-item>
                <v-row>
                    <v-col
                        cols="6"
                        v-for="(stat, idx) in content.getTaskExtraStats(task.taskName, task.mode)"
                        :key="idx"
                    >
                        <stat-container
                            class="pa-4"
                            :stats="[stat]"
                            :title="getStatTitle(stat)"
                        >
                            <template v-slot:default="{ data }">
                                <line-graph
                                    :series-data="data"
                                >
                                </line-graph>
                            </template>
                        </stat-container>
                    </v-col>
                </v-row>
            </v-tab-item>
        </template>
    </v-tabs>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { AimlabTaskData } from '@client/js/aimlab/aimlab_task'
import { AimlabContent, getAimlabContent } from '@client/js/aimlab/aimlab_content'
import StatLibrary from '@client/js/stats/stat_library'
import StatContainer from '@client/vue/stats/StatContainer.vue'
import LineGraph from '@client/vue/stats/LineGraph.vue'

@Component({
    components: {
        StatContainer,
        LineGraph
    }
})
export default class AimlabTaskPerformanceHistory extends Vue {
    @Prop({required: true})
    task! : AimlabTaskData

    content : AimlabContent = getAimlabContent()

    get hasExtraStats() : boolean {
        let stats = this.content.getTaskExtraStats(this.task.taskName, this.task.mode)
        return !!stats && stats.length > 0
    }

    getStatTitle(st : string) : string {
        if (!StatLibrary.exists(st)) {
            return 'Unknown'
        }
        return StatLibrary.getStatName(st)!
    }
}

</script>

<style scoped>

</style>