<template>
    <div class="pa-4">
        <v-row v-if="supported">
            <v-col cols="6">
                <v-text-field
                    :value="smallStep"
                    @change="changeSmallStep(parseInt(arguments[0]))"
                    solo
                    single-line
                    hide-details
                    type="number"
                >
                    <template v-slot:prepend>
                        Small Step (Arrow Keys)
                    </template>

                    <template v-slot:append>
                        ms
                    </template>
                </v-text-field>
            </v-col>

            <v-col cols="6">
                <v-text-field
                    :value="largeStep"
                    @change="changeLargeStep(parseInt(arguments[0]))"
                    solo
                    single-line
                    hide-details
                    type="number"
                >
                    <template v-slot:prepend>
                        Large Step (J/L)
                    </template>

                    <template v-slot:append>
                        ms
                    </template>
                </v-text-field>
            </v-col>
        </v-row>

        <div class="d-flex justify-center align-center" style="height: 600px;" v-else>
            <span class="text-h6">Not supported on this platform.</span>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'

@Component
export default class PlaybackSettingsItem extends Vue {
    get supported() {
///#if DESKTOP
        return true
///#else
        return false
///#endif
    }

    get smallStep(): number {
        return this.$store.state.settings.playback.smallStepSize
    }

    changeSmallStep(v: number) {
        this.$store.commit('changeSmallStep', v)
    }

    get largeStep(): number {
        return this.$store.state.settings.playback.largeStepSize
    }

    changeLargeStep(v: number) {
        this.$store.commit('changeLargeStep', v)
    }
}

</script>