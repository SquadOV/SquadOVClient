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
                        :champion-id="participant.championId"
                        :game-version="match.info.gameVersion"
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
import { LolParticipant } from '@client/js/lol/participant'
import { ddragonContainer } from '@client/js/lolDdragon'
import LolChampionIcon from '@client/vue/utility/lol/LolChampionIcon.vue'

@Component({
    components: {
        LolChampionIcon
    }
})
export default class LolParticipantDisplay extends Vue {
    @Prop({required: true})
    participant!: LolParticipant

    @Prop({required: true})
    match!: LolMatch

    @Prop({default: 64})
    widthHeight!: number

    @Prop({required: true})
    currentParticipantId!: number | null | undefined

    @Prop({type: Boolean, default: false})
    noTooltip!: boolean

    @Prop({type: Boolean, default: false})
    noBorder!: boolean

    name: string = ''

    @Watch('participant')
    refreshName() {
        this.name = ''
        this.name = this.participant.summonerName
    }

    mounted() {
        this.refreshName()
    }
    
    get iconStyle(): any {
        if (this.noBorder) {
            return {}
        }

        let borderColor: string
        let playerTeam = getTeamIdFromParticipantId(this.match, this.participant.participantId)

        if (!!this.currentParticipantId) {
            let currentTeam = getTeamIdFromParticipantId(this.match, this.currentParticipantId)
            if (this.participant.participantId === this.currentParticipantId) {
                borderColor = 'color-self'
            } else if (currentTeam === playerTeam) {
                borderColor = 'color-friendly'
            } else {
                borderColor = 'color-enemy'
            }
        } else {
            if (playerTeam === 100) {
                borderColor = 'color-blue-team'
            } else {
                borderColor = 'color-red-team'
            }
        }

        return {
            'border-radius': '4px',
            'border': `2px solid var(--${borderColor})`,
        }
    }
}

</script>