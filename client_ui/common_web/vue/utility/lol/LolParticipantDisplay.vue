<template>
    <div>
        <v-tooltip :disabled="noTooltip" bottom>
            <template v-slot:activator="{on, attrs}">
                <div
                    class="d-flex align-center"
                    v-on="on"
                    v-bind="attrs"
                >
                    <lol-champion-icon
                        :style="iconStyle"
                        :champion-id="participant.participant.championId"
                        :game-version="match.gameVersion"
                        :width="widthHeight"
                        :height="widthHeight"
                    >
                    </lol-champion-icon>

                    <div class="text-caption ml-1 font-weight-bold" v-if="noTooltip">
                        {{ name }}
                    </div>
                </div>
            </template>

            <div class="font-caption font-weight-bold">
                {{ name }}
            </div>
        </v-tooltip>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { LolMatch, getTeamIdFromParticipantId } from '@client/js/lol/matches'
import { WrappedLolParticipant } from '@client/js/lol/participant'
import { ddragonContainer } from '@client/js/lolDdragon'
import { Color, getGenericWinColor, getGenericLossColor, getGenericFirstPlaceColor, colorToCssString } from '@client/js/color'
import { getLolBlueTeamColor, getLolRedTeamColor } from '@client/js/lol/color'
import LolChampionIcon from '@client/vue/utility/lol/LolChampionIcon.vue'

@Component({
    components: {
        LolChampionIcon
    }
})
export default class LolParticipantDisplay extends Vue {
    @Prop({required: true})
    participant!: WrappedLolParticipant

    @Prop({required: true})
    match!: LolMatch

    @Prop({default: 64})
    widthHeight!: number

    @Prop({required: true})
    currentParticipantId!: number | null | undefined

    @Prop({type: Boolean, default: false})
    noTooltip!: boolean

    name: string = ''

    @Watch('participant')
    refreshName() {
        this.name = ''
        if (!!this.participant.identity?.player) {
            this.name = this.participant.identity.player.summonerName
        } else {
            ddragonContainer.getClientForVersion(this.match.gameVersion).getLolChampionName(this.participant.participant.championId).then((resp: string) => {
                this.name = resp
            }).catch((err: any) => {
                console.log('Failed to get LoL champion name as player name bakcup: ', err)
            })
        }
    }

    mounted() {
        this.refreshName()
    }
    
    get iconStyle(): any {
        let borderColor: Color
        let playerTeam = getTeamIdFromParticipantId(this.match, this.participant.participant.participantId)

        if (!!this.currentParticipantId) {
            let currentTeam = getTeamIdFromParticipantId(this.match, this.currentParticipantId)
            if (this.participant.participant.participantId === this.currentParticipantId) {
                borderColor = getGenericFirstPlaceColor()
            } else if (currentTeam === playerTeam) {
                borderColor = getGenericWinColor()
            } else {
                borderColor = getGenericLossColor()
            }
        } else {
            if (playerTeam === 100) {
                borderColor = getLolBlueTeamColor()
            } else {
                borderColor = getLolRedTeamColor()
            }
        }

        return {
            'border-radius': '4px',
            'border': `2px solid ${colorToCssString(borderColor)}`,
        }
    }
}

</script>