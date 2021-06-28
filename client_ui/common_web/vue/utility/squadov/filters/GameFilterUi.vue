<template>
    <v-select
        label="Games"
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
        v-if="value !== undefined"
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
import { SquadOvGames, gameToName, gameToIcon } from '@client/js/squadov/game'
import { Squad } from '@client/js/squadov/squad'

@Component
export default class GameFilterUi extends Vue {
    @Prop({type: Array})
    value!: SquadOvGames[] | undefined

    @Prop({type: Boolean, default: false})
    loading!: boolean

    @Prop({type: Boolean, default: false})
    readonly!: boolean

    get sortedValue(): SquadOvGames[] {
        if (!this.value) {
            return []
        }

        return this.value.sort((a: SquadOvGames, b: SquadOvGames) => {
            let aName = gameToName(a)
            let bName = gameToName(b)
            if (aName < bName) {
                return -1
            } else if (aName > bName) {
                return 1
            } else {
                return 0
            }
        })
    }

    changeValue(v: SquadOvGames[]) {
        if (v.length === 0) {
            this.$emit('input', undefined)
        } else {
            this.$emit('input', v)
        }
    }

    get items(): any[] {
        let supportedGames: SquadOvGames[] = [
            SquadOvGames.AimLab,
            SquadOvGames.Csgo,
            SquadOvGames.Hearthstone,
            SquadOvGames.LeagueOfLegends,
            SquadOvGames.TeamfightTactics,
            SquadOvGames.Valorant,
            SquadOvGames.WorldOfWarcraft
        ]
        return supportedGames.map((g: SquadOvGames) => ({
            text: gameToName(g),
            value: g,
            icon: gameToIcon(g)
        }))
    }
}

</script>

<style scoped>

.selection-checkbox {
    margin: 0px !important;
}

</style>