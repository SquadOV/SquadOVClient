<template>
    <div>
        <wow-filter-ui
            v-model="filters"
            for-instances
            class="my-2"
            :release="release"
            :saved-filter-loc="DataStorageLocation.WowInstance"
        >
        </wow-filter-ui>
        
        <loading-container :is-loading="!allInstances">
            <template v-slot:default="{ loading }">
                <div v-if="!loading">
                    <wow-instance-summary
                        class="mb-4"
                        v-for="(instance, idx) in allInstances"
                        :key="idx"
                        :instance="instance"
                        :user-id="userId"
                    >
                    </wow-instance-summary>

                    <v-btn
                        v-if="hasNext"
                        color="primary"
                        block
                        @click="loadMore"  
                    >
                        Load More
                    </v-btn>
                </div>
            </template>
        </loading-container>
    </div>
</template>

<script lang="ts">
import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, HalResponse, ApiData } from '@client/js/api'
import { WowInstance } from '@client/js/wow/matches'
import { WowMatchFilters, createEmptyWowMatchFilters } from '@client/js/wow/filters'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import WowFilterUi from '@client/vue/utility/wow/WowFilterUi.vue'
import WowInstanceSummary from '@client/vue/utility/wow/WowInstanceSummary.vue'
import { WowGameRelease } from '@client/js/staticData'
import { DataStorageLocation } from '@client/js/system/data_storage'

const maxTasksPerRequest : number = 10

@Component({
    components: {
        LoadingContainer,
        WowFilterUi,
        WowInstanceSummary,
    }
})
export default class WowInstanceGameLog extends Vue {
    DataStorageLocation = DataStorageLocation

    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    guid!: string

    @Prop({required: true})
    release!: WowGameRelease

    allInstances: WowInstance[] | null = null
    lastIndex: number = 0
    nextLink: string | null = null
    filters: WowMatchFilters = createEmptyWowMatchFilters()

    get hasNext() : boolean {
        return !!this.nextLink
    }

    @Watch('userId')
    @Watch('guid')
    @Watch('filters', {deep: true})
    refreshData() {
        this.allInstances = null
        this.lastIndex = 0
        this.nextLink = null
        this.loadMore()
    }

    loadMore() {
        if (!!this.allInstances && !this.nextLink) {
            return
        }

        apiClient.listWoWInstancesForCharacter({
            next: this.nextLink,
            userId: this.userId,
            guid: this.guid,
            start: this.lastIndex,
            end: this.lastIndex + maxTasksPerRequest,
            filters: this.filters,
        }).then((resp: ApiData<HalResponse<WowInstance[]>>) => {
            if (!this.allInstances) {
                this.allInstances = resp.data.data
            } else {
                this.allInstances.push(...resp.data.data)
            }
            this.lastIndex += resp.data.data.length
            if ("next" in resp.data._links) {
                this.nextLink = resp.data._links["next"].href
            } else {
                this.nextLink = null
            }
        }).catch((err: any) => {
            console.error('Failed to obtain WoW instances: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>