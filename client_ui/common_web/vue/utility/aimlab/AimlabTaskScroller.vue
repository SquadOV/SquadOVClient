<template>
    <div>
        <aimlab-task-summary-display
            class="mb-4"
            v-for="(task, index) in tasks"
            :task="task"
            :key="index"
            :user-id="userId"
        >
        </aimlab-task-summary-display>

        <v-btn
            v-if="canLoadMore"
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

@Component({
    components: {
        AimlabTaskSummaryDisplay
    }
})
export default class AimlabTaskScroller extends Vue {
    @Prop({type: Array, default: []})
    tasks! : AimlabTaskData[]

    @Prop({required: true})
    userId!: number

    @Prop({type: Boolean})
    canLoadMore!: boolean

    loadMore() {
        this.$emit('load-more')
    }
}

</script>

<style scoped>

</style>