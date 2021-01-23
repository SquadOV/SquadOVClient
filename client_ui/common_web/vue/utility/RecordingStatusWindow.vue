<template>
    <v-sheet v-if="hasSettings">
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
                <span :class="`mx-2 dot ${isRecording ? 'recording' : isPaused ? 'paused' : 'available'}`"></span>

                <span class="mx-2 white--text">
                    <span v-if="isRecording">
                        Recording
                    </span>

                    <span v-else-if="isPaused">
                        Paused
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
                :color="isPaused ? `success`: `warning`"
                @click="togglePause"
                :disabled="isRecording"
            >
                <v-icon v-if="isPaused">
                    mdi-play
                </v-icon>

                <v-icon v-else>
                    mdi-pause
                </v-icon>
            </v-btn>

            <div class="d-flex align-center mt-2">
                <span class="text-overline font-weight-bold mr-4">Quality:</span>
                <v-btn-toggle v-model="res" mandatory rounded dense>
                    <v-btn
                        v-for="res in resolutionItems"
                        :key="`res-${res}`"
                        :value="res"
                    >
                        {{ res }}p
                    </v-btn>
                </v-btn-toggle>
            </div>

            <div class="d-flex align-center mt-2">
                <span class="text-overline font-weight-bold mr-4">FPS:</span>
                <v-btn-toggle v-model="fps" mandatory rounded dense>
                    <v-btn
                        v-for="fps in fpsItems"
                        :key="`fps-${fps}`"
                        :value="fps"
                    >
                        {{ fps }}
                    </v-btn>
                </v-btn-toggle>
            </div>
        </div>
    </v-sheet>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import { ipcRenderer } from 'electron'

@Component
export default class RecordingStatusWindow extends Vue {
    expanded: boolean = false
    res: number = 1080
    fps: number = 60

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

    get isPaused(): boolean  {
        return this.$store.state.currentState.paused
    }

    get upDownIcon(): string {
        if (!this.expanded) {
            return 'mdi-chevron-up'
        } else {
            return 'mdi-chevron-down'
        }
    }

    get resolutionItems(): number[] {
        return [
            360,
            480,
            720,
            1080
        ]
    }

    get fpsItems(): number[] {
        return [
            30,
            60
        ]
    }

    @Watch('res')
    syncResolution() {
        this.$store.commit('changeRecordSettingRes', this.res)
    }

    @Watch('fps')
    syncFps() {
        this.$store.commit('changeRecordSettingFps', this.fps)
    }

    togglePause() {
        this.$store.commit('toggleRecordingPause')
        ipcRenderer.send('change-state-pause', this.isPaused)
    }

    get hasSettings(): boolean {
        return !!this.$store.state.settings
    }

    @Watch('hasSettings')
    reloadSettings() {
        if (!this.hasSettings) {
            return
        }
        this.res = this.$store.state.settings.record.resY
        this.fps = this.$store.state.settings.record.fps
    }

    mounted() {
        this.reloadSettings()
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

@keyframes record-pulse {
    0% {
        background-color: #121212;
    }

    50% {
        background-color: red;
    }
}

</style>