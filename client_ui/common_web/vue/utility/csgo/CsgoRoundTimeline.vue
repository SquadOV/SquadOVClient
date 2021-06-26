<template>
    <div class="d-flex align-end full-width">
        <template v-for="round in rounds">
            <csgo-single-round-timeline
                :match="match"
                :match-user-id="matchUserId"
                :round-data="round"
                :current-round="currentRound"
                @update:currentRound="$emit('update:currentRound', arguments[0])"
            >
            </csgo-single-round-timeline>
        </template>
    </div>
</template>

<script lang="ts">

import Component, { mixins } from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { CsgoFullMatchDataWrapper, CsgoEventRoundWrapper } from '@client/js/csgo/match'
import CsgoSingleRoundTimeline from '@client/vue/utility/csgo/CsgoSingleRoundTimeline.vue'
import CommonComponent from '@client/vue/CommonComponent'

@Component({
    components: {
        CsgoSingleRoundTimeline,
    }
})
export default class CsgoRoundTimeline extends mixins(CommonComponent) {
    @Prop({required: true})
    match!: CsgoFullMatchDataWrapper

    @Prop({required: true})
    matchUserId!: number | null

    @Prop({required: true})
    currentRound!: number

    get rounds(): CsgoEventRoundWrapper[] {
        return this.match.rounds
    }
}

</script>