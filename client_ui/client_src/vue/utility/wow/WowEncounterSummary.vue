<template>
    <wow-generic-match-summary
        :match="encounter"
        :user-id="userId"
        :elapsed-time="elapsedTime"
    >
        <div class="text-h6">
            {{ encounterName }}
        </div>

        <div class="d-flex align-center text-subtitle-2">
            {{ difficulty }} ({{ encounter.numPlayers }} Players)
        </div>
    </wow-generic-match-summary>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import WowGenericMatchSummary from '@client/vue/utility/wow/WowGenericMatchSummary.vue'
import { ApiData } from '@client/js/api'
import { Prop, Watch } from 'vue-property-decorator'
import { WowEncounter } from '@client/js/wow/matches'
import { WowDifficultyData } from '@client/js/wow/difficulty'
import { millisecondsToTimeString } from '@client/js/time'
import { staticClient } from '@client/js/staticData'
import axios from 'axios'

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

    difficultyData: WowDifficultyData | null = null

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

        return millisecondsToTimeString(finishTime.getTime() - this.encounter.tm.getTime())
    }

    @Watch('encounter')
    refreshDifficultyData() {
        this.difficultyData = null
        axios.get(staticClient.getWowDifficultyDataUrl(this.encounter.difficulty)).then((resp: ApiData<WowDifficultyData>) => {
            this.difficultyData = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain difficulty data: ', err)
        })
    }

    mounted() {
        this.refreshDifficultyData()
    }
}

</script>