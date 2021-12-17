<template>
    <div>
        <wow-generic-match-summary
            :match="encounter"
            :user-id="userId"
            :elapsed-time="elapsedTime"
            :mini="mini"
            :fill="fill"
            :disable-link="disableLink"
            :link-to-player-section="linkToPlayerSection"
            :access-token="accessToken"
            @go-to-character="$emit('go-to-character', arguments[0])"
        >
            <template v-slot="{ instanceName }">
                <div class="text-subtitle-1 font-weight-bold">
                    {{ encounterName }}
                </div>

                <div class="d-flex align-center text-subtitle-2">
                    {{ instanceName }} ({{ difficulty }} <span v-if="!!encounter.pullNumber">, Pull: {{ encounter.pullNumber }}</span>)
                </div>
            </template>

            <template v-slot:extra>
                <div class="d-flex align-center flex-wrap mt-1">
                    <div
                        v-for="(boss, idx) in bossesToDisplay"
                        :key="`boss-${idx}`"
                        class="d-flex align-center mr-2"
                    >
                        <span class="font-weight-bold">{{ boss.name }}</span>: {{ (boss.currentHp / boss.maxHp * 100.0).toFixed(1) }}%
                    </div>
                </div>
            </template>
        </wow-generic-match-summary>

        <div class="d-flex align-center" v-if="showPulls && !!pulls">
            <div class="mr-1 font-weight-bold">
                Pulls:
            </div>
            <v-pagination
                :value="currentPullPage"
                :length="pulls.length"
                @input="changePull(arguments[0] - 1)"
            >
            </v-pagination>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import WowGenericMatchSummary from '@client/vue/utility/wow/WowGenericMatchSummary.vue'
import { apiClient, ApiData } from '@client/js/api'
import { Prop, Watch } from 'vue-property-decorator'
import { WowBossStatus, WowEncounter } from '@client/js/wow/matches'
import { WowDifficultyData } from '@client/js/wow/difficulty'
import { WowInstanceData } from '@client/js/wow/instance'
import { secondsToTimeString } from '@client/js/time'
import { staticClient } from '@client/js/staticData'
import axios from 'axios'
import { WowMatchPageId } from '@client/js/pages'

@Component({
    components:{
        WowGenericMatchSummary
    }
})
export default class WowEncounterSummary extends Vue {
    @Prop({required: true})
    encounter!: WowEncounter

    @Prop({type: Number, required: true})
    userId!: number

    @Prop({type: Boolean, default: false})
    mini!: boolean

    @Prop({type: Boolean, default: false})
    fill!: boolean

    @Prop({type: Boolean, default: false})
    disableLink!: boolean

    @Prop({type: Boolean, default: false})
    linkToPlayerSection!: boolean

    @Prop()
    accessToken!: string | undefined

    @Prop({type: Boolean, default: false})
    showPulls!: boolean

    difficultyData: WowDifficultyData | null = null
    instanceData: WowInstanceData | null = null
    pulls: string[] | null = null

    get bossesToDisplay(): WowBossStatus[] {
        return this.encounter.boss.filter((ele: WowBossStatus) => {
            return !this.encounter.success && ele.currentHp !== null && ele.maxHp !== null
        })
    }

    get encounterName(): string {
        return this.encounter.encounterName.replace(/"/g, '')
    }

    get difficulty(): string {
        if (!this.difficultyData) {
            return 'Loading...'
        }
        return this.difficultyData.name
    }

    get elapsedTime(): string {
        let finishTime: Date = !!this.encounter.finishTime ?
            this.encounter.finishTime : new Date()

        return secondsToTimeString((finishTime.getTime() - this.encounter.tm.getTime()) / 1000)
    }

    get currentPullPage(): number {
        if (!this.pulls) {
            return 0
        }
        let idx = this.pulls.findIndex((ele) => ele === this.encounter.matchUuid)
        if (idx == -1) {
            return 0
        }
        return idx + 1
    }

    @Watch('encounter')
    refreshDifficultyData() {
        this.difficultyData = null
        axios.get(staticClient.getWowDifficultyDataUrl(this.encounter.build, this.encounter.difficulty)).then((resp: ApiData<WowDifficultyData>) => {
            this.difficultyData = resp.data
        }).catch((err: any) => {
            console.error('Failed to obtain difficulty data: ', err)
        })
    }

    @Watch('encounter')
    refreshPulls() {
        this.pulls = null

        if (!this.$store.state.currentUser) {
            return
        }

        apiClient.accessToken().getWowMatchPulls(this.encounter.matchUuid, this.userId).then((resp: ApiData<string[]>) => {
            this.pulls = resp.data
        }).catch((err: any) => {
            console.warn('Failed to get Wow match pulls: ', err)
        })
    }

    changePull(idx: number) {
        if (!this.pulls) {
            return
        }

        if (idx < 0 || idx >= this.pulls.length) {
            return
        }

        this.$router.push({
            name: WowMatchPageId,
            query: {
                userId: `${this.userId}`,
            },
            params: {
                matchUuid: this.pulls[idx],
            },
        })
    }

    mounted() {
        this.refreshDifficultyData()
        this.refreshPulls()
    }
}

</script>