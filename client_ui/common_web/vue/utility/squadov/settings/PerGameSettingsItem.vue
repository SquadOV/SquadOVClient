<template>
    <div class="full-width">
        <v-tabs>
            <v-tab>
                Enabled
            </v-tab>

            <v-tab-item>
                <disabled-supported-game-selector
                    :value="this.$store.state.settings.disabledGames"
                    @input="onChangeDisabledGames"
                >
                </disabled-supported-game-selector>
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
                            label="Record Battlegrounds"
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

                    <v-col cols="3">
                        <v-checkbox
                            class="ma-0 squeeze"
                            :input-value="$store.state.settings.games.wow.recordScenarios"
                            @change="$store.commit('changeWowRecordScenarios', arguments[0])"
                            label="Record Scenarios"
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

                                    Whether to record scenarios in World of Warcraft. Examples of this would be the Legion Timewalking Mage Tower Challenge.
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

                <v-row align="center">
                    <v-col cols="12">
                        <div class="text-h6 font-weight-bold mr-1">
                            Disabled Instances:
                        </div>

                        <wow-disabled-instances-display
                            class="mt-2"
                            :value="$store.state.settings.games.wow.doNotRecordInstances"
                            @input="$store.commit('changeWowDoNotRecordInstances', arguments[0])"
                        >
                        </wow-disabled-instances-display>
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
import DisabledSupportedGameSelector from '@client/vue/utility/squadov/DisabledSupportedGameSelector.vue'
import WowDisabledInstancesDisplay from '@client/vue/utility/squadov/settings/wow/WowDisabledInstancesDisplay.vue'
import { SquadOvGames } from '@client/js/squadov/game'

@Component({
    components: {
        DisabledSupportedGameSelector,
        WowDisabledInstancesDisplay,
    }
})
export default class PerGameSettingsItem extends Vue {
    onChangeDisabledGames(disabled: SquadOvGames[]) {
        this.$store.commit('changeDisabledGames', disabled)
    }
}

</script>