<template>
    <v-sheet
        class="d-flex flex-column justify-center pl-4"
        rounded
        :style="style"
        width="100%"
        height="120px"
    >
        <div class="d-flex match-details-header">
            <span class="text-h4 mr-4">Match Details</span>
            <span class="text-overline">
                {{ matchTime }} 
            </span>
        </div>

        <div>
            <span class="text-subtitle">{{ queueType }} ({{ mapName }})</span>
        </div>
    </v-sheet>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { getGameMode, getIsCustom } from '@client/js/valorant/valorant_matches'
import { ValorantMatchDetailsWrapper } from '@client/js/valorant/valorant_matches_parsed'
import { getValorantContent } from '@client/js/valorant/valorant_content'
import format from 'date-fns/format'

@Component
export default class ValorantMatchHeaderSummary extends Vue {
    @Prop({ required: true})
    match! : ValorantMatchDetailsWrapper

    @Prop()
    currentPlayer!: string | null

    get winLossColor() : string {
        if (!this.currentPlayer) {
            return 'color-self'
        } else {
            let won = this.match.getPlayerTeam(this.currentPlayer)
            return won ?  'color-top-place' : 'color-bottom-place'
        }
    }

    get mapName() : string {
        if (!this.match._details.data.matchInfo.mapId) {
            return 'Unknown'
        }

        let cnt = getValorantContent(null)
        return cnt.mapAssetPathToName(this.match._details.data.matchInfo.mapId)
    }

    get queueType() : string {
        let queue = getGameMode(this.match._details.data.matchInfo.gameMode, this.match._details.data.matchInfo.isRanked)
        if (!queue) {
            return 'Unknown'
        }

        if (getIsCustom(this.match._details.data.matchInfo.provisioningFlowId)) {
            queue = `[Custom] ${queue}`
        }
        return queue!
    }

    get style() : any {
        let ret : any = {
            'border-left': `5px solid var(--${this.winLossColor})`,
            'background-position': 'right',
            'background-size': 'contain',
        }

        //@ts-ignore
        ret['background-image'] = `linear-gradient(to right, #1E1E1E 0 70%, transparent), url(${this.$root.generateAssetUri(`assets/valorant/maps/preview/${this.mapName}.jpg`)})`
        return ret
    }

    get matchTime() : string {
        if (!this.match._details.data.matchInfo.serverStartTimeUtc) {
            return 'Unknown'
        }
        return format(this.match._details.data.matchInfo.serverStartTimeUtc, 'MMMM do uuuu, h:mm a')
    }

}

</script>

<style scoped>

.match-details-header {
    align-items: flex-end;
}

</style>