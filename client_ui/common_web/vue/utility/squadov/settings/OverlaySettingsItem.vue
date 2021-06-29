<template>
    <div>
        <div class="d-flex align-center mt-4">
            <div class="font-weight-bold text-h6">Overlays (Experimental)</div>
            <v-spacer></v-spacer>
            <v-btn
                v-if="!enabled"
                color="success"
                @click="toggleEnable(true)"
            >
                Enable
            </v-btn>

            <v-btn
                v-else
                color="error"
                @click="toggleEnable(false)"
            >
                Disable
            </v-btn>

            <v-tooltip bottom max-width="450px">
                <template v-slot:activator="{on, attrs}">
                    <v-icon v-on="on" v-bind="attrs">
                        mdi-help-circle
                    </v-icon>
                </template>

                Enable our experimental overlays feature if you want to add overlays to your recorded videos.
                This allows you to, for example, hide chat in the games you play.
                To get started, select the game you want to preview the overlay for, run that game, hit 'Start Preview', and edit the overlay layers as needed.
                Feedback on how to improve this tool would be much appreciated!
            </v-tooltip>
        </div>
        <v-divider class="my-4"></v-divider>

        <template v-if="enabled">
            <!-- Overlay preview -->
            <div class="d-flex justify-center">
                <div class="preview-container">
                    <video :class="`video-js vjs-fluid`" ref="video">
                    </video>

                    <template v-if="canEdit">
                        <video-draw-overlay
                            v-for="(layer, idx) in editableLayers"
                            :key="`overlay-${idx}`"
                            :ref="`overlay-${idx}`"
                            :inactive="isLayerDisabled(layer) || selectedLayerIdx !== idx"
                            :hidden="isLayerDisabled(layer)"
                            v-model="layer.fabric"
                            :z-index="startZIndex - 4 * idx"
                            @canvassize="onChangeCanvasSize(layer, arguments[0], arguments[1])"
                            disable-brush
                            disable-line
                            disable-text
                            disable-blur
                            simple-shapes
                        >
                        </video-draw-overlay>
                    </template>
                </div>
            </div>

            <!-- Overlay preview control -->
            <div class="d-flex justify-center align-center mt-2">
                <v-select
                    v-model="selectedGame"
                    hide-details
                    label="Game"
                    :items="gameItems"
                    dense
                    :disabled="previewStarted"
                    outlined
                    style="max-width: 500px;"
                >
                </v-select>

                <v-btn
                    class="ml-2"
                    color="success"
                    v-if="!previewStarted"
                    @click="startPreview"
                >
                    Start Preview
                </v-btn>

                <v-btn
                    class="ml-2"
                    color="primary"
                    v-if="!previewPlaying && previewStarted"
                    @click="togglePreview(true)"
                >
                    Resume Preview (F8)
                </v-btn>

                <v-btn
                    class="ml-2"
                    color="warning"
                    v-if="previewPlaying"
                    @click="togglePreview(false)"
                >
                    Pause Preview (F8)
                </v-btn>

                <v-btn
                    class="ml-2"
                    color="error"
                    v-if="previewStarted"
                    @click="stopPreview"
                >
                    Stop Preview
                </v-btn>

                <v-btn
                    class="ml-2"
                    color="success"
                    v-if="previewStarted && ready && !canEdit"
                    @click="startEdit"
                >
                    Edit
                </v-btn>

                <v-btn
                    class="ml-2"
                    color="success"
                    v-if="canEdit"
                    @click="saveEdit"
                >
                    Save
                </v-btn>

                <v-btn
                    class="ml-2"
                    color="success"
                    v-if="canEdit"
                    @click="cancelEdit"
                >
                    Cancel
                </v-btn>
            </div>

            <div class="d-flex align-center justify-center mt-1 font-caption">
                It make take a few seconds for the preview to update with your changes.
            </div>

            <!-- Creation control -->
            <v-container fluid>
                <v-row>
                    <v-col cols="4">
                        <div class="d-flex align-center">
                            <div class="font-subtitle-2 font-weight-bold">
                                Layers
                            </div>

                            <v-spacer></v-spacer>

                            <v-btn
                                color="primary"
                                @click="newLayer"
                                small
                                icon
                                :disabled="!canEdit"
                            >
                                <v-icon>
                                    mdi-plus
                                </v-icon>
                            </v-btn>
                        </div>
                        <v-divider class="my-2"></v-divider>
                        <v-list-item-group dense v-model="selectedLayerIdx">
                            <v-list-item
                                v-for="(layer, idx) in editableLayers"
                                :key="`layer-${idx}`"
                                :value="idx"
                            >
                                <v-list-item-icon>
                                    <v-icon small v-if="isLayerDisabled(layer)">
                                        mdi-eye-off
                                    </v-icon>

                                    <v-icon small v-else>
                                        mdi-eye
                                    </v-icon>
                                </v-list-item-icon>
                                <v-list-item-content>
                                    <v-list-item-title>
                                        {{ layer.name }}
                                    </v-list-item-title>
                                </v-list-item-content>

                                <v-spacer></v-spacer>

                                <v-btn
                                    color="success"
                                    small
                                    icon
                                    @click.stop="layerUp(idx)"
                                    :disabled="idx == 0"
                                >
                                    <v-icon>
                                        mdi-arrow-up-box
                                    </v-icon>
                                </v-btn>

                                <v-btn
                                    class="ml-1"
                                    color="error"
                                    small
                                    icon
                                    @click.stop="layerDown(idx)"
                                    :disabled="idx == (editableLayers.length - 1)"
                                >
                                    <v-icon>
                                        mdi-arrow-down-box
                                    </v-icon>
                                </v-btn>
                            </v-list-item>
                        </v-list-item-group>
                    </v-col>

                    <v-col cols="8">
                        <div v-if="!!selectedLayer">
                            <v-text-field
                                v-model="selectedLayer.name"
                                hide-details
                                outlined
                                single-line
                                dense
                                label="Name"
                                :readonly="!canEdit"
                            >
                            </v-text-field>

                            <game-filter-ui
                                v-model="selectedLayer.games"
                                class="mt-4"
                                :readonly="!canEdit"
                            >
                            </game-filter-ui>
                        </div>
                    </v-col>
                </v-row>
            </v-container>
        </template>

        <v-snackbar
            v-model="editSuccess"
            :timeout="5000"
            color="success"
        >
            You've successfully saved your changed to the overlay! It may take a few seconds for the updates to be reflected in the preview.
        </v-snackbar>

        <v-snackbar
            v-model="previewStartMsg"
            :timeout="5000"
            color="success"
        >
            It may take a few seconds for the preview to load. If the preview does not show up please ensure that the selected game is currently running. One moment please...
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import VideoDrawOverlay from '@client/vue/utility/VideoDrawOverlay.vue'
import GameFilterUi from '@client/vue/utility/squadov/filters/GameFilterUi.vue'
import videojs from 'video.js'
import 'video.js/dist/video-js.css' 

/// #if DESKTOP
import { ipcRenderer } from 'electron'
/// #endif

import { allGames, gameToName, SquadOvGames } from '@client/js/squadov/game'
import { createEmptyOverlay, SquadOvOverlay } from '@client/js/system/settings'

// The gist of how this component works is that when the user
// just wants to see a preview of their overlays, we just display a HLS stream.
// When the user goes to edit the preview, we render a canvas based preview
// of the overlays on top of the video and turn off the overlays in the C++ side.
@Component({
    components: {
        VideoDrawOverlay,
        GameFilterUi,
    }
})
export default class OverlaySettingsItem extends Vue {
    get startZIndex(): number {
        return 30 + this.editableLayers.length * 4
    }

    previewStarted: boolean = false
    previewPlaying: boolean = false

    selectedGame: SquadOvGames = SquadOvGames.AimLab
    player: videojs.Player | null = null

    editableLayers: SquadOvOverlay[] = []
    selectedLayerIdx: number | null = null
    canEdit: boolean = false
    editSuccess: boolean = false
    previewStartMsg: boolean = false
    ready: boolean = false

    layerUp(idx: number) {
        if (idx <= 0 || idx >= this.editableLayers.length) {
            return
        }

        let layer = this.editableLayers[idx]
        this.editableLayers.splice(idx, 1)
        this.editableLayers.splice(idx-1, 0, layer)
    }

    layerDown(idx: number) {
        if (idx < 0 || idx >= this.editableLayers.length - 1) {
            return
        }

        let layer = this.editableLayers[idx]
        this.editableLayers.splice(idx, 1)
        this.editableLayers.splice(idx+1, 0, layer)
    }

    onChangeCanvasSize(layer: SquadOvOverlay, width: number, height: number) {
        layer.width = width
        layer.height = height
    }

    get isLayerDisabled(): (layer: SquadOvOverlay) => boolean {
        return (layer: SquadOvOverlay) => {
            return !layer.games.includes(this.selectedGame)
        }
    }

    get selectedLayer(): SquadOvOverlay | null {
        if (this.selectedLayerIdx === null) {
            return null
        }

        if (this.selectedLayerIdx < 0 || this.selectedLayerIdx >= this.editableLayers.length) {
            return null
        }

        return this.editableLayers[this.selectedLayerIdx]
    }

    @Watch('layers')
    syncLayers() {
        this.editableLayers = JSON.parse(JSON.stringify(this.layers))
    }

    newLayer() {
        this.editableLayers.push(createEmptyOverlay(`Layer ${this.editableLayers.length + 1}`))
    }

    startEdit() {
        this.canEdit = true

        // Need to disable the overlay in the preview being generated from the C++ side.
///#if DESKTOP
        ipcRenderer.send('enable-record-preview-overlays', false)
///#endif
    }

    saveEdit() {
        this.canEdit = false

        // We rely on the changeOverlayLayers function doing an immediate save to disk of the settings.
        // Because after the save is done, we need to tell the preview to reload.
        this.$store.commit('changeOverlayLayers', this.editableLayers)

        // Reload preview so that we get the new overlay layers.
///#if DESKTOP
        ipcRenderer.send('reload-record-preview')
///#endif

        this.resetSource()
        this.editSuccess = true
    }

    cancelEdit() {
        this.canEdit = false
///#if DESKTOP
        ipcRenderer.send('enable-record-preview-overlays', true)
///#endif
        this.syncLayers()
    }

    $refs!: {
        [layerName: string]: VideoDrawOverlay | HTMLVideoElement
        video: HTMLVideoElement
    }

    get gameItems(): any[] {
        return allGames.map((ele: SquadOvGames) => ({
            text: gameToName(ele),
            value: ele,
        }))
    }

    get enabled(): boolean {
        return this.$store.state.settings.record.overlays.enabled
    }

    get layers(): SquadOvOverlay[] {
        return this.$store.state.settings.record.overlays.layers
    }

    toggleEnable(v: boolean) {
        this.$store.commit('changeOverlayEnabled', v)
    }

    resetSource() {
        if (!!this.player) {
            this.player.reset()
            this.player.src([])

            this.player.src([{
                src: 'http://localhost:9999/live/preview/index.m3u8',
                type: 'application/x-mpegURL',
            }])
        }
    }

    retryTimeout: any = null
    startPreview() {
        this.previewStarted = true
        this.previewPlaying = true
        this.ready = false
        this.previewStartMsg = true
    
///#if DESKTOP
        ipcRenderer.invoke('start-record-preview', this.selectedGame).then(() => {
            if (!this.player) {
                this.player = videojs(this.$refs.video, {
                    controls: false,
                    autoplay: true,
                    preload: 'auto',
                    muted: true,
                })

                this.player.on('canplay', () => {
                    this.ready = true
                })

                this.player.on('error', (err: any) => {
                    console.log('On VideoJS Preview Error...trying again soon')
                    if (this.retryTimeout !== null) {
                        return
                    }

                    this.retryTimeout = window.setTimeout(() => {
                        this.retryTimeout = null

                        if (!this.previewStarted || !this.previewPlaying) {
                            return
                        }
                        

                        this.resetSource()
                    }, 3000)
                })
            }

            this.resetSource()
        })
///#endif
    }

    stopPreview() {
        this.previewStarted = false
        this.previewPlaying = false

        if (!!this.player) {
            this.player.src([])
            this.player.reset()
        }

///#if DESKTOP
        ipcRenderer.send('stop-record-preview')
///#endif
    }

    togglePreview(v: boolean) {
        if (!this.previewStarted || !this.player) {
            return
        }

        this.previewPlaying = v
        if (!this.previewPlaying) {
            this.player.pause()
        } else {
            this.player.play()
            this.player.liveTracker.seekToLiveEdge()
        }
    }

    mounted() {
        this.syncLayers()
    }

    beforeDestroy() {
        if (!!this.player) {
            this.player.dispose()
        }
        this.player = null
    }
}

</script>

<style scoped>

.preview-container {
    position: relative;
    width: 80%;
}

</style>