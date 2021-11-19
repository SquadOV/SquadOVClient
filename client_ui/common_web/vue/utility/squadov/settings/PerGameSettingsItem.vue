<template>
    <div class="full-width">
        <v-tabs>
            <v-tab>
                Enabled
            </v-tab>

            <v-tab-item>
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
            </v-tab-item>

            <v-tab>
                WoW
            </v-tab>

            <v-tab-item>
                <v-row align="center">
                    <v-col cols="4">
                        <v-checkbox
                            class="ma-0 squeeze"
                            :input-value="$store.state.settings.games.wow.useCombatLogTimeout"
                            @change="$store.commit('changeWowLogTimeout', {use: arguments[0], seconds: $store.state.settings.games.wow.timeoutSeconds2})"
                            label="Use Combat Log Timeout"
                            hide-details
                            dense
                        >
                            <template v-slot:append>
                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>

                                    When enabled, SquadOV will detect extended periods of time with no new data in the combat log as being the same as a match end.
                                    Use this when you find SquadOV recording well past the end of a match (generally, until and/or including the next match).
                                    We recommend you use a fairly large value here (2-3 minutes).
                                </v-tooltip>
                            </template>
                        </v-checkbox>
                    </v-col>

                    <v-col cols="8">
                        <v-text-field
                            class="squeeze"
                            :value="$store.state.settings.games.wow.timeoutSeconds2"
                            @change="$store.commit('changeWowLogTimeout', {use: $store.state.settings.games.wow.useCombatLogTimeout, seconds: parseInt(arguments[0])})"
                            label="Timeout"
                            solo
                            single-line
                            dense
                            hide-details
                            :disabled="!$store.state.settings.games.wow.useCombatLogTimeout"
                            type="number"
                            style="max-width: 450px"
                        >
                            <template v-slot:append>
                                seconds
                            </template>
                        </v-text-field>
                    </v-col>
                </v-row>

                <v-row align="center">
                    <v-col cols="3">
                        <v-checkbox
                            class="ma-0 squeeze"
                            :input-value="$store.state.settings.games.wow.recordArenas"
                            @change="$store.commit('changeWowRecordArenas', arguments[0])"
                            label="Record Arenas"
                            hide-details
                            dense
                        >
                            <template v-slot:append>
                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>

                                    Whether to record arenas (2v2, 3v3, Skirmish, etc.) in World of Warcraft.
                                </v-tooltip>
                            </template>
                        </v-checkbox>
                    </v-col>

                     <v-col cols="3">
                        <v-checkbox
                            class="ma-0 squeeze"
                            :input-value="$store.state.settings.games.wow.recordBattlegrounds"
                            @change="$store.commit('changeWowRecordBattlegrounds', arguments[0])"
                            label="Record Arenas"
                            hide-details
                            dense
                        >
                            <template v-slot:append>
                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>

                                    Whether to record battlegrounds in World of Warcraft.
                                </v-tooltip>
                            </template>
                        </v-checkbox>
                    </v-col>

                    <v-col cols="3">
                        <v-checkbox
                            class="ma-0 squeeze"
                            :input-value="$store.state.settings.games.wow.recordDungeons"
                            @change="$store.commit('changeWowRecordDungeons', arguments[0])"
                            label="Record Dungeons"
                            hide-details
                            dense
                        >
                            <template v-slot:append>
                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>

                                    Whether to record dungeons in World of Warcraft. If the run turns out to be a keystone, we will convert the run to record a keystone (challenge).
                                </v-tooltip>
                            </template>
                        </v-checkbox>
                    </v-col>

                    <v-col cols="3">
                        <v-checkbox
                            class="ma-0 squeeze"
                            :input-value="$store.state.settings.games.wow.recordKeystones"
                            @change="$store.commit('changeWowRecordKeystones', arguments[0])"
                            label="Record Keystones"
                            hide-details
                            dense
                        >
                            <template v-slot:append>
                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>

                                    Whether to record keystones in World of Warcraft.
                                </v-tooltip>
                            </template>
                        </v-checkbox>
                    </v-col>

                    <v-col cols="3">
                        <v-checkbox
                            class="ma-0 squeeze"
                            :input-value="$store.state.settings.games.wow.recordEncounters"
                            @change="$store.commit('changeWowRecordEncounters', arguments[0])"
                            label="Record Encounters"
                            hide-details
                            dense
                        >
                            <template v-slot:append>
                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>

                                    Whether to record encounters (boss fights) in raids and non-keystone dungeons in World of Warcraft.
                                </v-tooltip>
                            </template>
                        </v-checkbox>
                    </v-col>
                </v-row>

                <v-row align="center">
                    <v-col cols="6">
                        <div class="d-flex align-center">
                            <div class="font-weight-bold mr-1">
                                Minimum Duration:
                            </div>

                            <v-text-field
                                class="squeeze"
                                :value="$store.state.settings.games.wow.minimumTimeSecondsToRecord"
                                @change="$store.commit('changeWowMinimumTimeSecondsToRecord', parseInt(arguments[0]))"
                                label="Minimum Time to Record"
                                solo
                                single-line
                                dense
                                hide-details
                                type="number"
                                style="max-width: 450px"
                            >
                                <template v-slot:append>
                                    seconds

                                    <v-tooltip bottom max-width="450px">
                                        <template v-slot:activator="{on, attrs}">
                                            <v-icon v-on="on" v-bind="attrs">
                                                mdi-help-circle
                                            </v-icon>
                                        </template>

                                        When recording instances (RBGs, WoW classic arenas, full raids), this is the number of seconds
                                        the recording needs to be for SquadOV to not discard it. This will not take effect for regular encounters,
                                        keystones, and retail arena recordings.
                                    </v-tooltip>
                                </template>
                            </v-text-field>
                        </div>
                    </v-col>
                </v-row>
            </v-tab-item>

            <v-tab>
                Valorant
            </v-tab>

            <v-tab-item>
                <v-row align="center">
                    <v-col cols="3">
                        <v-checkbox
                            class="ma-0 squeeze"
                            :input-value="$store.state.settings.games.valorant.recordStandard"
                            @change="$store.commit('changeValorantRecordingSettings', {standard: arguments[0]})"
                            label="Record Standard"
                            hide-details
                            dense
                        >
                            <template v-slot:append>
                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>

                                    Record standard Valorant games.
                                </v-tooltip>
                            </template>
                        </v-checkbox>
                    </v-col>

                    <v-col cols="3">
                        <v-checkbox
                            class="ma-0 squeeze"
                            :input-value="$store.state.settings.games.valorant.recordSpikeRush"
                            @change="$store.commit('changeValorantRecordingSettings', {spikeRush: arguments[0]})"
                            label="Record Spike Rush"
                            hide-details
                            dense
                        >
                            <template v-slot:append>
                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>

                                    Record spike rush Valorant games.
                                </v-tooltip>
                            </template>
                        </v-checkbox>
                    </v-col>

                    <v-col cols="3">
                        <v-checkbox
                            class="ma-0 squeeze"
                            :input-value="$store.state.settings.games.valorant.recordDeathmatch"
                            @change="$store.commit('changeValorantRecordingSettings', {deathmatch: arguments[0]})"
                            label="Record Deathmatch"
                            hide-details
                            dense
                        >
                            <template v-slot:append>
                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>

                                    Record spike rush Valorant games.
                                </v-tooltip>
                            </template>
                        </v-checkbox>
                    </v-col>

                    <v-col cols="3">
                        <v-checkbox
                            class="ma-0 squeeze"
                            :input-value="$store.state.settings.games.valorant.recordOther"
                            @change="$store.commit('changeValorantRecordingSettings', {other: arguments[0]})"
                            label="Record Other"
                            hide-details
                            dense
                        >
                            <template v-slot:append>
                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>

                                    Record Valorant games played with any other game mode.
                                </v-tooltip>
                            </template>
                        </v-checkbox>
                    </v-col>
                </v-row>
            </v-tab-item>
        </v-tabs>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { SquadOvGames, allGames, gameToRect, gameToName } from '@client/js/squadov/game'

@Component
export default class PerGameSettingsItem extends Vue {
    SquadOvGames = SquadOvGames
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
        let disabled = new Set(this.$store.state.settings.disabledGames)
        return allGames.filter((ele: SquadOvGames) => !disabled.has(ele))
    }

    onChangeEnabledGames(enabled: SquadOvGames[]) {
        let enabledSet = new Set(enabled)
        let disabled = allGames.filter((ele: SquadOvGames) => !enabledSet.has(ele))
        this.$store.commit('changeDisabledGames', disabled)
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