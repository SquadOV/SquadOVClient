<template>
    <div>
        <h1>BOOP</h1>
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

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, HalResponse, ApiData } from '@client/js/api'
import { WowEncounter } from '@client/js/wow/matches'

const maxTasksPerRequest : number = 20

@Component
export default class WowEncounterGameLog extends Vue {
    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    guid!: string

    allEncounters: WowEncounter[] | null = null
    lastIndex: number = 0
    nextLink: string | null = null

    get hasNext() : boolean {
        return !!this.nextLink
    }

    @Watch('userId')
    @Watch('guid')
    refreshData() {
        this.allEncounters = null
        this.lastIndex = 0
        this.nextLink = null
        this.loadMore()
    }

    loadMore() {
        if (!!this.allEncounters && !this.nextLink) {
            return
        }
        apiClient.listWoWEncountersForCharacter({
            next: this.nextLink,
            userId: this.userId,
            guid: this.guid,
            start: this.lastIndex,
            end: this.lastIndex + maxTasksPerRequest,
        }).then((resp: ApiData<HalResponse<WowEncounter[]>>) => {
            if (!this.allEncounters) {
                this.allEncounters = resp.data.data
            } else {
                this.allEncounters.push(...resp.data.data)
            }
            this.lastIndex += resp.data.data.length
            if ("next" in resp.data._links) {
                this.nextLink = resp.data._links["next"].href
            } else {
                this.nextLink = null
            }
        }).catch((err: any) => {
            console.log('Failed to obtain WoW encounters: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>