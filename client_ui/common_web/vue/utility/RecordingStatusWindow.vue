<template>
    <v-sheet>
        <v-btn
            color="secondary"
            block
            depressed
            text
            tile
            @click="expanded = !expanded"
        >
            <v-icon>
                {{ upDownIcon }}
            </v-icon>

            <v-spacer></v-spacer>

            <div class="d-flex align-center">
                <span :class="`mx-2 dot ${isRecording ? 'recording' : isPaused ? 'paused' : isStorageFull ? 'storageError' : 'available'}`"></span>

                <span class="mx-2 white--text">
                    <span v-if="isRecording">
                        Recording
                    </span>

                    <span v-else-if="isPaused">
                        Paused
                    </span>

                    <span v-else-if="isStorageFull">
                        Uh oh. Low storage!
                    </span>

                    <span v-else-if="isInGame">
                        In-Game
                    </span>

                    <span v-else>
                        Available
                    </span>
                </span>

                <v-img
                    class="mx-2"
                    v-for="game in gameIconsToShow"
                    :key="`${game}-icon`"
                    contain
                    width="24px"
                    :src="$root.generateAssetUri(`assets/${game}-logo.png`)"
                >
                </v-img>
            </div>

            <v-spacer></v-spacer>

            <v-icon>
                {{ upDownIcon }}
            </v-icon>
        </v-btn>

        <div class="pa-2" v-if="expanded">
            <v-btn
                block
                icon
                tile
                :color="isPaused ? `success` : `warning`"
                @click="togglePause"
                v-if="!isRecording"
            >
                <v-icon v-if="isPaused">
                    mdi-play
                </v-icon>

                <v-icon v-else>
                    mdi-pause
                </v-icon>
            </v-btn>

            <v-btn
                block
                icon
                tile
                color="error"
                @click="requestStopRecording"
                v-else
            >
                <v-icon>
                    mdi-stop
                </v-icon>
            </v-btn>

            <recording-settings-item mini ref="record"></recording-settings-item>
        </div>

        <v-dialog
            v-model="showHideStopConfirm"
            max-width="40%"
            persistent
        >
            <v-card>
                <v-card-title>
                    Are you sure you wish to manually stop recording?
                </v-card-title>
                <v-divider class="mb-4"></v-divider>

                <v-card-text>
                    For all games except World of Warcraft, <span class="font-weight-bold">you will lose this recorded VOD.</span>
                    For World of Warcraft, manually stopping the recording will cause your run to be marked as a loss (encounters, keystones, arenas).
                    You may lose some combat log data.
                    It is recommended you let SquadOV finish recording on its own.
                </v-card-text>

                <v-card-actions>
                    <v-checkbox
                        :input-value="$store.state.settings.record.needConfirmManualStop"
                        @change="$store.commit('changeNeedConfirmManualStop', arguments[0])"
                        dense
                        hide-details
                        label="Show this popup every time."
                    >
                    </v-checkbox>

                    <v-btn
                        class="ml-2"
                        color="success"
                        @click="showHideStopConfirm = false"
                    >
                        Nevermind!
                    </v-btn>

                    <v-spacer></v-spacer>

                    <v-btn
                        color="error"
                        @click="forceStopRecording"
                    >
                        Stop Recording
                    </v-btn>
                </v-card-actions>
            </v-card>
        </v-dialog>
    </v-sheet>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import { ipcRenderer } from 'electron'
import { AudioDeviceListingResponse } from '@client/js/system/audio'
import RecordingSettingsItem from '@client/vue/utility/squadov/settings/RecordingSettingsItem.vue'

@Component({
    components: {
        RecordingSettingsItem
    }
})
export default class RecordingStatusWindow extends Vue {
    expanded: boolean = false
    res: number = 1080
    fps: number = 60

    outputOptions: string[] = []
    selectedOutput: string = ''
    defaultOutput: string = ''
    outputVolume: number = 1.0

    inputOptions: string[] = []
    selectedInput: string = ''
    defaultInput: string = ''
    inputVolume: number = 1.0
    showHideStopConfirm: boolean = false
    storageGBLeft: number = 0
    isStorageFull: boolean = false

    $refs!: {
        record: RecordingSettingsItem
    }

    @Watch('expanded')
    onToggleExpanded() {
        if (this.expanded) {
            Vue.nextTick(() => {
                this.$refs.record.refreshFeatureFlags()
            })
        }
    }

    get gameIconsToShow(): string[] {
        if (this.isRecording) {
            return this.$store.state.currentState.recordingGames
        } else {
            return this.$store.state.currentState.runningGames
        }
    }

    get isRecording(): boolean {
        return this.$store.state.currentState.recordingGames.length > 0
    }

    get isInGame(): boolean {
        return this.$store.state.currentState.runningGames.length > 0
    }

    get isPaused(): boolean {
        return this.$store.state.currentState.paused
    }

    get upDownIcon(): string {
        if (!this.expanded) {
            return 'mdi-chevron-up'
        } else {
            return 'mdi-chevron-down'
        }
    }

    get maxLocalRecordingSizeGb(): number {
        return this.$store.state.settings.record.maxLocalRecordingSizeGb
    }

    get localDiskSpaceRecordUsageGb(): number | null {
///#if DESKTOP
        return this.$store.state.localDiskSpaceRecordUsageGb
///#else
        return null
///#endif
    }

    @Watch('$store.state.settings.record.useLocalRecording')
    @Watch('localDiskSpaceRecordUsageGb')
    @Watch('maxLocalRecordingSizeGb')
    checkLocalDiskSpaceRecordUsage() {
        // First check if user is using local recording or if localDiskSpaceRecordUsageGb is still null
        if (!this.$store.state.settings.record.useLocalRecording) {
            this.isStorageFull = false
            return
        } else if (this.localDiskSpaceRecordUsageGb === null) {
            return
        }
        this.storageGBLeft = this.maxLocalRecordingSizeGb - this.localDiskSpaceRecordUsageGb
        // If storage is less than 1 GB, we set the Recording status to show low storage warning.
        if (this.storageGBLeft < 1) {
            this.isStorageFull = true
        } else {
            this.isStorageFull = false
        }
    }

    togglePause() {
        this.$store.commit('toggleRecordingPause')
        ipcRenderer.send('change-state-pause', this.isPaused)
    }

    requestStopRecording() {
        if (!!this.$store.state.settings.record.needConfirmManualStop) {
            this.showHideStopConfirm = true
        } else {
            this.forceStopRecording()
        }
    }

    forceStopRecording() {
        this.showHideStopConfirm = false
        ipcRenderer.send('force-stop-recording')
    }
}

</script>

<style scoped>
.dot {
    height: 24px;
    width: 24px;
    border: 2px solid white;
    border-radius: 50%;
    display: inline-block;
}

.dot.recording {
    animation: 2s linear 0s infinite normal record-pulse;
}

.dot.paused {
    background-color: yellow;
}

.dot.available {
    background-color: green;
}

.dot.storageError {
    background-color: darkred;
}

@keyframes record-pulse {
    0% {
        background-color: #121212;
    }

    50% {
        background-color: red;
    }
}
</style>