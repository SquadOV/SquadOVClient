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

            <div>
                Status: No Recording
            </div>

            <v-spacer></v-spacer>

            <v-icon>
                {{ upDownIcon }}
            </v-icon>
        </v-btn>

        <div class="pa-2" v-if="expanded">
            <div class="d-flex align-center">
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

@Component
export default class RecordingStatusWindow extends Vue {
    expanded: boolean = false
    res: number = 1080
    fps: number = 60

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

    mounted() {
        this.res = this.$store.state.settings.record.resY
        this.fps = this.$store.state.settings.record.fps
    }
}

</script>