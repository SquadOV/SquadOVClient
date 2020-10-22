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
            return '#1E1E1E'
        } else {
            let won = this.match.getPlayerTeam(this.currentPlayer)
            return won ?  '#4CAF50' : '#FF5252'
        }
    }

    get mapName() : string {
        let cnt = getValorantContent(this.match._details.patchId)
        return cnt.mapAssetPathToName(this.match._details.map)
    }

    get queueType() : string {
        let queue = getGameMode(this.match._details.patchId, this.match._details.gameMode, this.match._details.isRanked)
        if (getIsCustom(this.match._details.provisioningFlowId)) {
            queue = `[Custom] ${queue}`
        }
        return queue!
    }

    get style() : any {
        return {
            'border-left': `5px solid ${this.winLossColor}`,
            'background-image': `linear-gradient(to right, #1E1E1E 0 70%, transparent), url(assets/valorant/maps/preview/${this.mapName}.png)`,
            'background-position': 'right',
            'background-size': 'contain',
        }
    }

    get matchTime() : string {
        return format(this.match._details.matchTime, 'MMMM do uuuu, h:mm a')
    }

}

</script>

<style scoped>

.match-details-header {
    align-items: flex-end;
}

</style>