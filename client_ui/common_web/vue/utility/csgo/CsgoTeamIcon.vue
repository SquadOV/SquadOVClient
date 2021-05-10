<template>
    <v-img
        v-if="isValid"
        :width="widthHeight"
        :max-width="widthHeight"
        :height="widthHeight"
        :max-height="widthHeight"
        :src="$root.generateAssetUri(src)"
        contain
    >
    </v-img>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { CsgoTeam } from '@client/js/csgo/events'

@Component
export default class CsgoTeamIcon extends Vue {
    @Prop({required: true})
    team!: CsgoTeam

    @Prop({default: 48})
    widthHeight!: number

    get src(): string {
        if (this.team == CsgoTeam.CT) {
            return 'assets/csgo/teams/teamlogo-large-ct.png'
        } else {
            return 'assets/csgo/teams/teamlogo-large-terrorist.png'
        }
    }

    get isValid(): boolean {
        return this.team !== CsgoTeam.Spectate
    }
}

</script>