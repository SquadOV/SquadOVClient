<template>
    <wow-generic-match-summary
        :match="arena"
        :user-id="userId"
        :elapsed-time="elapsedTime"
        :mini="mini"
        :fill="fill"
        use-teams
        :friendly-team="friendlyTeam"
        :disable-link="disableLink"
        :link-to-player-section="linkToPlayerSection"
        :access-token="accessToken"
        @go-to-character="$emit('go-to-character', arguments[0])"
    >
        <template v-slot="{ instanceName }">
            <div class="text-subtitle-1 font-weight-bold">
                {{ instanceName }} ({{ arenaType }})
            </div>

            <div class="d-flex align-center text-subtitle-2" :style="ratingColorStyle">
                <v-icon class="mr-1" color="success" x-small v-if="arena.success">
                    mdi-arrow-up-bold
                </v-icon>

                <v-icon class="mr-1" color="error" x-small v-else>
                    mdi-arrow-down-bold
                </v-icon>
                {{ rating }}
            </div>
        </template>
    </wow-generic-match-summary>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import WowGenericMatchSummary from '@client/vue/utility/wow/WowGenericMatchSummary.vue'
import { Prop } from 'vue-property-decorator'
import { WowArena, getOppositeWowArenaTeam } from '@client/js/wow/matches'
import { secondsToTimeString } from '@client/js/time'

@Component({
    components: {
        WowGenericMatchSummary,
    }
})
export default class WowArenaSummary extends Vue {
    @Prop({required: true})
    arena!: WowArena

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

    get arenaType(): string {
        if (this.arena.type === '5v5') {
            return 'War Game'
        } else {
            return this.arena.type
        }
    }

    get friendlyTeam(): number {
        if (this.arena.winningTeamId === null) {
            return 0
        }
        return this.arena.success ? this.arena.winningTeamId : getOppositeWowArenaTeam(this.arena.winningTeamId)
    }

    get ratingColorStyle() : any {
        return {
            'color': `var(--${this.arena.success ? 'color-top-place' : 'color-bottom-place'})`
        }
    }

    get rating(): number {
        if (!this.arena.newRatings) {
            return 0
        }
        return this.arena.newRatings[this.friendlyTeam]
    }

    get elapsedTime(): string {
        let finishTime: Date = !!this.arena.finishTime ?
            this.arena.finishTime : new Date()

        return secondsToTimeString((finishTime.getTime() - this.arena.tm.getTime()) / 1000)
    }
}

</script>