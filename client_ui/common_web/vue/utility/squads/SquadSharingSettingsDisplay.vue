<template>
    <loading-container :is-loading="!settings">
        <template v-slot:default="{ loading }">
            <v-tabs vertical v-if="!loading">
                <v-tab>
                    Games
                </v-tab>

                <v-tab-item>
                    <disabled-supported-game-selector
                        v-model="settings.disabledGames"
                        @input="saveSharingSettings"
                    >
                    </disabled-supported-game-selector>
                </v-tab-item>

                <v-tab>
                    WoW
                </v-tab>

                <v-tab-item>
                    <v-container fluid>
                        <v-row align="center">
                            <v-col cols="3">
                                <v-checkbox
                                    class="ma-0 squeeze"
                                    v-model="settings.wow.disableArenas"
                                    @change="saveSharingSettings"
                                    label="Block Arenas"
                                    hide-details
                                    dense
                                >
                                </v-checkbox>
                            </v-col>

                            <v-col cols="3">
                                <v-checkbox
                                    class="ma-0 squeeze"
                                    v-model="settings.wow.disableBgs"
                                    @change="saveSharingSettings"
                                    label="Block Battlegrounds"
                                    hide-details
                                    dense
                                >
                                </v-checkbox>
                            </v-col>

                            <v-col cols="3">
                                <v-checkbox
                                    class="ma-0 squeeze"
                                    v-model="settings.wow.disableDungeons"
                                    @change="saveSharingSettings"
                                    label="Block Dungeons"
                                    hide-details
                                    dense
                                >
                                </v-checkbox>
                            </v-col>

                            <v-col cols="3">
                                <v-checkbox
                                    class="ma-0 squeeze"
                                    v-model="settings.wow.disableKeystones"
                                    @change="saveSharingSettings"
                                    label="Block Keystones"
                                    hide-details
                                    dense
                                >
                                </v-checkbox>
                            </v-col>

                            <v-col cols="3">
                                <v-checkbox
                                    class="ma-0 squeeze"
                                    v-model="settings.wow.disableEncounters"
                                    @change="saveSharingSettings"
                                    label="Block Encounters"
                                    hide-details
                                    dense
                                >
                                </v-checkbox>
                            </v-col>
                        </v-row>
                    </v-container>
                </v-tab-item>
            </v-tabs>

            <v-snackbar
                v-model="updateError"
                :timeout="5000"
                color="error"
            >
                Failed to update squad sharing settings.
            </v-snackbar>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { SquadSharingSettings } from '@client/js/squadov/squad'
import { apiClient, ApiData } from '@client/js/api'
import DisabledSupportedGameSelector from '@client/vue/utility/squadov/DisabledSupportedGameSelector.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'

@Component({
    components: {
        DisabledSupportedGameSelector,
        LoadingContainer,
    }
})
export default class SquadSharingSettingsDisplay extends Vue {
    @Prop({required: true})
    squadId!: number

    @Prop({type: Boolean, default: false})
    isOwner!: boolean

    settings: SquadSharingSettings | null = null
    updateError: boolean = false

    @Watch('squadId')
    refreshData() {
        this.settings = null
        apiClient.getSquadSharingSettings(this.squadId).then((resp: ApiData<SquadSharingSettings>) => {
            this.settings = resp.data
        }).catch((err: any) => {
            console.error('Failed to get squad sharing settings: ' , err)
        })
    }

    saveSharingSettings() {
        if (!this.settings) {
            return
        }

        apiClient.updateSquadSharingSettings(this.squadId, this.settings).catch((err: any) => {
            this.updateError = true
            console.error('Failed to update squad sharing settings: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>