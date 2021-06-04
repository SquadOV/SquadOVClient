<template>
    <div class="d-flex flex-column align-center flex-shrink-1 flex-grow-1">
        <v-img
            v-for="k in numRegularKills"
            :key="`norm-${k}`"
            :height="16"
            :max-height="16"
            :width="16"
            :max-width="16"
            contain
            :src="$root.generateAssetUri('assets/csgo/misc/kill_icon.png')"
        >
        </v-img>

        <v-img
            v-for="k in numHsKills"
            :key="`hs-${k}`"
            :height="16"
            :max-height="16"
            :width="16"
            :max-width="16"
            contain
            :src="$root.generateAssetUri('assets/csgo/misc/kill_icon_headshot.png')"
        >
        </v-img>

        <v-btn
            tile
            :class="`round-button full-width flex-grow-1 flex-shrink-1 ${ (currentRound == roundData.roundNum) ? 'selected-round' : '' }`"
            :style="style"
            @click="$emit('update:currentRound', roundData.roundNum)"
        >
            <span class="text-center">{{ roundData.roundNum + 1 }}</span>
            <v-icon color="yellow" v-if="isMvp">
                mdi-star
            </v-icon>
        </v-btn>

        <v-img
            :height="16"
            :max-height="16"
            :width="16"
            :max-width="16"
            contain
            :src="$root.generateAssetUri('assets/csgo/misc/status-dead.png')"
            v-if="hasDeath"
        >
        </v-img>

        <div class="dead-placeholder" v-else>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { CsgoFullMatchDataWrapper, CsgoEventRoundWrapper } from '@client/js/csgo/match'
import { CsgoRoundPlayerStats } from '@client/js/csgo/events'
import { CsgoTeam } from '@client/js/csgo/events'

@Component
export default class CsgoSingleRoundTimeline extends Vue {
    @Prop({required: true})
    match!: CsgoFullMatchDataWrapper

    @Prop({required: true})
    matchUserId!: number | null

    @Prop({required: true})
    roundData!: CsgoEventRoundWrapper

    @Prop({required: true})
    currentRound!: number

    get userTeam(): CsgoTeam {
        if (this.matchUserId === null) {
            return CsgoTeam.Spectate
        }
        return this.roundData.userTeam(this.matchUserId)
    }

    get userStats(): CsgoRoundPlayerStats | undefined {
        if (this.matchUserId === null) {
            return undefined
        }
        return this.roundData.userStats(this.matchUserId)
    }

    get numHsKills(): number {
        if (!this.userStats || !this.userStats.headshotKills) {
            return 0
        }
        return this.userStats.headshotKills
    }

    get numRegularKills(): number {
        if (!this.userStats) {
            return 0
        }

        if (!!this.userStats.headshotKills) {
            return this.userStats.kills - this.userStats.headshotKills
        } else {
            return this.userStats.kills
        }
    }

    get isMvp(): boolean {
        if (this.matchUserId === null) {
            return false
        }
        return this.roundData.isUserMvp(this.matchUserId)
    }

    get hasDeath(): boolean {
        if (this.matchUserId === null) {
            return false
        }
        return this.roundData.didUserDie(this.matchUserId)
    }

    get style(): any {
        let isWinner = this.matchUserId === null ? false : this.roundData.isUserWinner(this.matchUserId)

        return {
            'background-color': `var(--${this.userTeam === CsgoTeam.CT ? 'color-csgo-ct' : 'color-csgo-t'})`,
            'border-top': `2px solid var(--${isWinner ? 'color-first-place' : 'color-bottom-place'})`,
            'height': '36px',
            'max-height': '36px',
        }
    }
}

</script>

<style scoped>

.round-button {
    border-left: 1px solid rgba(255, 255, 255, 0.5);
    border-right: 1px solid rgba(255, 255, 255, 0.5);
    min-width: 0px !important;
}

.selected-round {
    background-color: rgba(255, 255, 0, 0.5) !important;
}

.dead-placeholder {
    height: 16px;
    max-height: 16px;
    width: 1px;
}

</style>