<template>
    <div>
        <aimlab-task-summary-display
            class="mb-4"
            v-for="(task, index) in limitedTasks"
            :task="task"
            :key="index"
        >
        </aimlab-task-summary-display>

        <v-btn
            v-if="!loadedAllTasks"
            color="primary"
            block
            @click="loadMore"  
        >
            Load More
        </v-btn>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { AimlabTaskData } from '@client/js/aimlab/aimlab_task'
import AimlabTaskSummaryDisplay from '@client/vue/utility/aimlab/AimlabTaskSummaryDisplay.vue'

const loadMoreTasks: number = 20

@Component({
    components: {
        AimlabTaskSummaryDisplay
    }
})
export default class AimlabTaskScroller extends Vue {
    @Prop({type: Array, default: []})
    tasks! : AimlabTaskData[]

    maxTaskCount: number = loadMoreTasks

    get loadedAllTasks() : boolean {
        return this.maxTaskCount >= this.tasks.length
    }

    get limitedTasks() : AimlabTaskData[] {
        return this.tasks.slice(0, this.maxTaskCount)
    }

    loadMore() {
        this.maxTaskCount += loadMoreTasks
    }
}

</script>

<style scoped>

</style>