<template>
    <v-select
        label="WoW Release"
        :value="sortedValue"
        @input="changeValue"
        :items="items"
        deletable-chips
        chips
        multiple
        :clearable="!readonly"
        outlined
        hide-details
        dense
        :loading="loading"
        :readonly="readonly"
    >
        <template v-slot:item="{ item }">
            <div class="d-flex full-width align-center">
                <v-checkbox
                    class="selection-checkbox"
                    dense
                    hide-details
                    :input-value="!!value && value.includes(item.value)"
                    readonly
                >
                </v-checkbox>

                {{ item.text }}

                <v-spacer></v-spacer>

                <v-img
                    :src="$root.generateAssetUri(item.icon)"
                    :width="32"
                    :max-width="32"
                    :height="32"
                    :max-height="32"
                    contain
                >
                </v-img>
            </div>
        </template>
    </v-select>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { SquadOvWowRelease, wowReleaseToGame, gameToIcon, wowReleaseToName } from '@client/js/squadov/game'

@Component
export default class WowReleaseFilterUi extends Vue {
    @Prop({type: Array})
    value!: SquadOvWowRelease[] | undefined

    @Prop({type: Boolean, default: false})
    loading!: boolean

    @Prop({type: Boolean, default: false})
    readonly!: boolean

    get sortedValue(): SquadOvWowRelease[] {
        if (!this.value) {
            return []
        }

        return this.value.sort((a: SquadOvWowRelease, b: SquadOvWowRelease) => {
            let aName = wowReleaseToName(a)
            let bName = wowReleaseToName(b)
            if (aName < bName) {
                return -1
            } else if (aName > bName) {
                return 1
            } else {
                return 0
            }
        })
    }

    changeValue(v: SquadOvWowRelease[]) {
        if (v.length === 0) {
            this.$emit('input', undefined)
        } else {
            this.$emit('input', v)
        }
    }

    get items(): any[] {
        let releases: SquadOvWowRelease[] = [
            SquadOvWowRelease.Retail,
            SquadOvWowRelease.Vanilla,
            SquadOvWowRelease.Tbc,
        ]

        return releases.map((r: SquadOvWowRelease) => {
            let g = wowReleaseToGame(r)
            return {
                text: wowReleaseToName(r),
                value: r,
                icon: gameToIcon(g)
            }
        })
    }
}

</script>

<style scoped>

.selection-checkbox {
    margin: 0px !important;
}

</style>