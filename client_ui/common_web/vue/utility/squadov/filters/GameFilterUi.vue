<template>
    <v-select
        label="Games"
        :value="value"
        @input="changeValue"
        :items="items"
        deletable-chips
        chips
        multiple
        clearable
        outlined
        hide-details
        dense
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

@Component
export default class GameFilterUi extends Vue {
    @Prop({type: Array})
    value!: SquadOvGames[] | undefined

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
            SquadOvGames.Hearthstone,
        ]

        if (this.$store.state.features.enableLol) {
            supportedGames.push(SquadOvGames.LeagueOfLegends)
        }

        if (this.$store.state.features.enableTft) {
            supportedGames.push(SquadOvGames.TeamfightTactics)
        }

        supportedGames.push(SquadOvGames.Valorant, SquadOvGames.WorldOfWarcraft)

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