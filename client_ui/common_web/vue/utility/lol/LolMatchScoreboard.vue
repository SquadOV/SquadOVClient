<template>
    <div>
        <lol-match-team-scoreboard
            v-for="(team, idx) in match.info.teams"
            :key="idx"
            :match="match"
            :current-participant-id="currentParticipantId"
            :team-id="team.teamId"
        >
        </lol-match-team-scoreboard>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { LolMatch } from '@client/js/lol/matches'
import LolMatchTeamScoreboard from '@client/vue/utility/lol/LolMatchTeamScoreboard.vue'

@Component({
    components: {
        LolMatchTeamScoreboard
    }
})
export default class LolMatchScoreboard extends Vue {
    @Prop({required: true})
    match!: LolMatch
    
    @Prop({required: true})
    currentParticipantId!: number | null | undefined
}

</script>