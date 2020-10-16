<template>
    <loading-container :is-loading="!allTasks">
        <template v-slot:default="{ loading }">
            <div v-if="!loading">
                <aimlab-task-scroller
                    :tasks="allTasks"
                    v-if="allTasks.length > 0"
                    :can-load-more="hasNext"
                    @load-more="loadMoreData"
                >
                </aimlab-task-scroller>

                <div class="text-center" v-else>
                    <span class="text-h5">No Aim Lab tasks found.</span>
                </div>
            </div>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { AimlabTaskData } from '@client/js/aimlab/aimlab_task'
import { apiClient, HalResponse, ApiData } from '@client/js/api'
import AimlabTaskScroller from '@client/vue/utility/aimlab/AimlabTaskScroller.vue'

const maxTasksPerRequest : number = 20

@Component({
    components: {
        LoadingContainer,
        AimlabTaskScroller
    }
})
export default class AimlabGameLog extends Vue {
    allTasks : AimlabTaskData[] | null = null
    lastIndex: number = 0
    nextLink: string | null = null

    get hasNext() : boolean {
        return !!this.nextLink
    }

    loadMoreData() {
        if (!!this.allTasks && !this.nextLink) {
            return
        }

        let user = this.$store.state.currentUser!
        apiClient.allAimlabTaskData({
            next: this.nextLink,
            userId: user.id,
            start: this.lastIndex,
            end: this.lastIndex + maxTasksPerRequest
        }).then((resp : ApiData<HalResponse<AimlabTaskData[]>>) => {
            if (!this.allTasks) {
                this.allTasks = resp.data.data
            } else {
                this.allTasks.push(...resp.data.data)
            }
            this.lastIndex += resp.data.data.length
            if ("next" in resp.data._links) {
                this.nextLink = resp.data._links["next"].href
            } else {
                this.nextLink = null
            }
        }).catch((err : any) => {
            console.log('Failed to list Aim Lab tasks: ' + err);
        })
    }

    mounted() {
        this.loadMoreData()
    }
}

</script>

<style scoped>

</style>