<template>
    <div>
        <aimlab-filter-ui
            v-model="filters"
            class="my-2"
            :saved-filter-loc="DataStorageLocation.Aimlab"
        >
        </aimlab-filter-ui>
        
        <loading-container :is-loading="!allTasks">
            <template v-slot:default="{ loading }">
                <div v-if="!loading">                
                    <aimlab-task-scroller
                        :tasks="allTasks"
                        v-if="allTasks.length > 0"
                        :can-load-more="hasNext"
                        :user-id="userId"
                        @load-more="loadMoreData"
                    >
                    </aimlab-task-scroller>

                    <div class="text-center" v-else>
                        <span class="text-h5">No Aim Lab tasks found.</span>
                    </div>
                </div>
            </template>
        </loading-container>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { AimlabTaskData } from '@client/js/aimlab/aimlab_task'
import { AimlabMatchFilters, createEmptyAimlabMatchFilters } from '@client/js/aimlab/filters'
import { apiClient, HalResponse, ApiData } from '@client/js/api'
import AimlabTaskScroller from '@client/vue/utility/aimlab/AimlabTaskScroller.vue'
import AimlabFilterUi from '@client/vue/utility/aimlab/AimlabFilterUi.vue'
import { DataStorageLocation } from '@client/js/system/data_storage'

const maxTasksPerRequest : number = 20

@Component({
    components: {
        LoadingContainer,
        AimlabTaskScroller,
        AimlabFilterUi,
    }
})
export default class AimlabGameLog extends Vue {
    DataStorageLocation = DataStorageLocation

    @Prop({required: true})
    userId!: number
    filters: AimlabMatchFilters = createEmptyAimlabMatchFilters()

    allTasks : AimlabTaskData[] | null = null
    lastIndex: number = 0
    nextLink: string | null = null

    get hasNext() : boolean {
        return !!this.nextLink
    }

    @Watch('userId')
    @Watch('filters', { deep: true })
    refreshData() {
        this.allTasks = null
        this.nextLink = null
        this.lastIndex = 0
        this.loadMoreData()
    }

    loadMoreData() {
        if (!!this.allTasks && !this.nextLink) {
            return
        }

        apiClient.allAimlabTaskData({
            next: this.nextLink,
            userId: this.userId,
            start: this.lastIndex,
            end: this.lastIndex + maxTasksPerRequest,
            filters: this.filters,
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
            console.error('Failed to list Aim Lab tasks: ' + err);
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>

<style scoped>

</style>