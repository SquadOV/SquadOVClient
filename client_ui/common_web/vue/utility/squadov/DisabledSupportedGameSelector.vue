<template>
    <v-item-group class="d-flex flex-wrap align-center justify-center full-width" multiple :value="enabledGames" @change="onChangeEnabledGames">
        <v-item
            v-for="(game, idx) in allGames"
            :key="idx"
            :value="game"
            v-slot="{active, toggle}"
        >
            <div
                @click="toggle"
                :style="gameStyle(game)"
                :class="`game ${active ? 'enabled-game' : 'disabled-game'} my-2 mx-2`"
            >
                <div class="text-h6 font-weight-bold game-title">
                    {{ gameToName(game) }}

                    <v-icon class="ml-2" :color="`${active ? 'success' : 'error'}`">
                        {{ active ? 'mdi-check-circle' : 'mdi-close-circle' }}
                    </v-icon>
                </div>
            </div>
        </v-item>
    </v-item-group>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { SquadOvGames, allGames, gameToName, gameToRect } from '@client/js/squadov/game'

@Component
export default class DisabledSupportedGameSelector extends Vue {
    @Prop({required: true})
    value!: SquadOvGames[]

    allGames = allGames.sort((a: SquadOvGames, b: SquadOvGames) => {
        let an = gameToName(a)
        let bn = gameToName(b)
        if (an < bn) {
            return -1
        } else if (an > bn) {
            return 1
        } else {
            return 0
        }
    })

    gameToName = gameToName

    gameStyle(game: SquadOvGames): any {
        return {
            //@ts-ignore
            'background-image': `url('${this.$root.generateAssetUri(gameToRect(game))}')`,
            'background-size': 'cover',
            'width': '30%',
            'height': '200px',
        }
    }

    get enabledGames(): SquadOvGames[] {
        let disabled = new Set(this.value)
        return allGames.filter((ele: SquadOvGames) => !disabled.has(ele))
    }

    onChangeEnabledGames(enabled: SquadOvGames[]) {
        let enabledSet = new Set(enabled)
        let disabled = allGames.filter((ele: SquadOvGames) => !enabledSet.has(ele))
        this.$emit('input', disabled)
    }
}

</script>

<style scoped>

.game {
    transition: opacity 0.5s linear 0s;
    cursor: pointer;
}

.enabled-game {
    opacity: 1.0;
}

.disabled-game {
    opacity: 0.3;
}

.game-title {
    position: relative;
    top: 70%;
    left: 5%;
}

</style>