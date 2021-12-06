<template>
    <!-- One tab per wow release -->
    <v-tabs>
        <template v-for="(release, idx) in ALL_WOW_RELEASES">
            <v-tab
                :key="`tab-${idx}`"
            >
                <v-img
                    class="mr-1"
                    width="32px"
                    :src="$root.generateAssetUri(gameToIcon(wowReleaseToGame(release)))"
                    contain
                >
                </v-img>

                {{ wowReleaseToName(release) }}
            </v-tab>

            <v-tab-item
                :key="`item-${idx}`"
            >
                <wow-release-disabled-instances-display
                    :release="release"
                    :value="instancesForRelease(release)"
                    @input="changeForRelease(release, arguments[0])"
                >
                </wow-release-disabled-instances-display>
            </v-tab-item>
        </template>
    </v-tabs>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { WowDisabledInstance } from '@client/js/system/settings'
import { ALL_WOW_RELEASES, wowReleaseToGame, wowReleaseToName, gameToIcon, SquadOvWowRelease } from '@client/js/squadov/game'
import WowReleaseDisabledInstancesDisplay from '@client/vue/utility/squadov/settings/wow/WowReleaseDisabledInstancesDisplay.vue'

@Component({
    components: {
        WowReleaseDisabledInstancesDisplay
    }
})
export default class WowDisabledInstancesDisplay extends Vue {
    ALL_WOW_RELEASES = ALL_WOW_RELEASES
    gameToIcon = gameToIcon
    wowReleaseToGame = wowReleaseToGame
    wowReleaseToName = wowReleaseToName

    @Prop({required: true})
    value!: WowDisabledInstance[]

    get instancesForRelease(): (r: SquadOvWowRelease) => WowDisabledInstance[] {
        return (r: SquadOvWowRelease) => {
            return this.value.filter((i: WowDisabledInstance) => i.release === r)
        }
    }

    changeForRelease(r: SquadOvWowRelease, val: WowDisabledInstance[]) {
        let newVal = [...this.value.filter((i: WowDisabledInstance) => i.release !== r), ...val]
        this.$emit('input', newVal)
    }
}

</script>