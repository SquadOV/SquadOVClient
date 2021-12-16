<template>
    <div>
        <v-alert
            v-if="showAlert"
            class="pa-2 ma-0"
            type="info"
            dismissible
            tile
            width="100%"
            @input="onClose"
        >
            <template v-slot:prepend>
                <v-icon>mdi-exclamation</v-icon>
            </template>

            <div class="d-flex align-center">
                <div>
                    Just a friendly reminder that you have disabled the recording of {{ createDisplayStringFromList(disabledGameNames) }}!
                </div>

                <v-spacer></v-spacer>

                <v-btn
                    small
                    class="ml-2"
                    color="secondary"
                    :to="settingsTo"
                >
                    Change Settings
                </v-btn>
            </div>
        </v-alert>
    </div>
</template>

<script lang="ts">
import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import { createDisplayStringFromList } from '@client/js/strings'
import { SettingsPageId } from '@client/js/pages'
import { gameToName, SquadOvGames } from '@client/js/squadov/game'

@Component
export default class DisabledGamesAlert extends Vue {
    showAlert: boolean = false
    createDisplayStringFromList = createDisplayStringFromList
    onClose() {
        this.muteAlert()
    }

    muteAlert() {
        window.sessionStorage.setItem('muteDisabledGamesAlert', 'true')
    }

    get isAlarmMute() {
        return (window.sessionStorage.getItem('muteDisabledGamesAlert') == 'true' ? true : false)
    }

    get disabledGameNames(): string[] {
        return this.$store.state.settings.disabledGames.map((ele: SquadOvGames) => gameToName(ele))
    }

    @Watch('$store.state.settings.disabledGames')
    resync() {
        if (!this.isAlarmMute) {
            this.showAlert = this.$store.state.settings.disabledGames.length > 0
        }
    }

    mounted() {
        this.resync()
    }

    get settingsTo(): any {
        return {
            name: SettingsPageId,
            query: {
                inputTab: 1,
            }
        }
    }
}
</script>
