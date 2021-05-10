<template>
    <div class="d-flex align-center justify-space-around">
        <steam-account-display
            v-if="kill.killer !== null"
            :width="height"
            :height="height"
            :account="match.steamAccount(match.userIdToSteamId(kill.killer))"
            :style="userStyling(kill.killer)"
        >
        </steam-account-display>

        <template v-if="kill.assister !== null">
            <v-icon small class="mx-2">
                mdi-plus
            </v-icon>

            <steam-account-display
                :width="height"
                :height="height"
                :account="match.steamAccount(match.userIdToSteamId(kill.assister))"
                :style="userStyling(kill.assister)"
            >
            </steam-account-display>
        </template>

        <csgo-weapon-icon
            class="ml-4"
            v-if="kill.weapon !== null"
            :weapon="kill.weapon"
            :height="height"
        >
        </csgo-weapon-icon>

        <v-img
            class="ml-1"
            v-if="!!kill.headshot"
            :width="height"
            :max-width="height"
            :height="height"
            :max-height="height"
            :src="$root.generateAssetUri('assets/csgo/misc/icon_headshot.svg')"
        >
        </v-img>

        <v-img
            class="ml-1"
            v-if="!!kill.smoke"
            :width="height"
            :max-width="height"
            :height="height"
            :max-height="height"
            :src="$root.generateAssetUri('assets/csgo/misc/smoke_kill.svg')"
        >
        </v-img>

        <v-img
            class="ml-1"
            v-if="!!kill.blind"
            :width="height"
            :max-width="height"
            :height="height"
            :max-height="height"
            :src="$root.generateAssetUri('assets/csgo/misc/blind_kill.svg')"
        >
        </v-img>

        <v-img
            class="ml-1"
            v-if="!!kill.wallbang"
            :width="height"
            :max-width="height"
            :height="height"
            :max-height="height"
            :src="$root.generateAssetUri('assets/csgo/misc/penetrate.svg')"
        >
        </v-img>

        <v-img
            class="ml-1"
            v-if="!!kill.noscope"
            :width="height"
            :max-width="height"
            :height="height"
            :max-height="height"
            :src="$root.generateAssetUri('assets/csgo/misc/noscope.svg')"
        >
        </v-img>

        <steam-account-display
            class="ml-4"
            v-if="kill.victim !== null"
            :width="32"
            :height="32"
            :account="match.steamAccount(match.userIdToSteamId(kill.victim))"
            :style="userStyling(kill.victim)"
        >
        </steam-account-display>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { CsgoEventRoundWrapper, CsgoFullMatchDataWrapper } from '@client/js/csgo/match'
import { CsgoRoundKill, CsgoTeam } from '@client/js/csgo/events'
import { Color, getGenericWinColor, getGenericLossColor, getGenericFirstPlaceColor, colorToCssString } from '@client/js/color'
import SteamAccountDisplay from '@client/vue/utility/steam/SteamAccountDisplay.vue'
import CsgoWeaponIcon from '@client/vue/utility/csgo/CsgoWeaponIcon.vue'

@Component({
    components: {
        SteamAccountDisplay,
        CsgoWeaponIcon,
    }
})
export default class CsgoRoundKillDisplay extends Vue {
    @Prop({required: true})
    match!: CsgoFullMatchDataWrapper

    @Prop({required: true})
    kill!: CsgoRoundKill

    @Prop({required: true})
    height!: number

    @Prop({required: true})
    matchUserId!: number | null

    @Prop({required: true})
    roundData!: CsgoEventRoundWrapper

    userStyling(userId: number): any {
        let color: Color = getGenericLossColor()
        let refTeam: CsgoTeam = this.roundData.userTeam(!!this.matchUserId ? this.matchUserId : 0)
        let inputTeam: CsgoTeam = this.roundData.userTeam(userId)
        if (userId === this.matchUserId) {
            color = getGenericFirstPlaceColor()
        } else if (refTeam === inputTeam) {
            color = getGenericWinColor()
        } else {
            color = getGenericLossColor()
        }

        return {
            'border': `2px solid ${colorToCssString(color)}`
        }
    }
}

</script>