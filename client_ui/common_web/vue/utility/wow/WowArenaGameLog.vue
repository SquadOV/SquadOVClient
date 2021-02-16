<template>
    <loading-container :is-loading="!allArenas">
        <template v-slot:default="{ loading }">
            <div v-if="!loading">
                <wow-arena-summary
                    class="mb-4"
                    v-for="(arena, idx) in allArenas"
                    :key="idx"
                    :arena="arena"
                    :user-id="userId"
                >
                </wow-arena-summary>

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
</template>

<script lang="ts">
import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, HalResponse, ApiData } from '@client/js/api'
import { WowArena } from '@client/js/wow/matches'
import WowArenaSummary from '@client/vue/utility/wow/WowArenaSummary.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'

const maxTasksPerRequest : number = 10

@Component({
    components: {
        LoadingContainer,
        WowArenaSummary,
    }
})
export default class WowArenaGameLog extends Vue {
    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    guid!: string

    allArenas: WowArena[] | null = null
    lastIndex: number = 0
    nextLink: string | null = null

    get hasNext() : boolean {
        return !!this.nextLink
    }

    @Watch('userId')
    @Watch('guid')
    refreshData() {
        this.allArenas = null
        this.lastIndex = 0
        this.nextLink = null
        this.loadMore()
    }

    loadMore() {
        if (!!this.allArenas && !this.nextLink) {
            return
        }
        apiClient.listWoWArenasForCharacter({
            next: this.nextLink,
            userId: this.userId,
            guid: this.guid,
            start: this.lastIndex,
            end: this.lastIndex + maxTasksPerRequest,
        }).then((resp: ApiData<HalResponse<WowArena[]>>) => {
            if (!this.allArenas) {
                this.allArenas = resp.data.data
            } else {
                this.allArenas.push(...resp.data.data)
            }
            this.lastIndex += resp.data.data.length
            if ("next" in resp.data._links) {
                this.nextLink = resp.data._links["next"].href
            } else {
                this.nextLink = null
            }
        }).catch((err: any) => {
            console.log('Failed to obtain WoW arenas: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>