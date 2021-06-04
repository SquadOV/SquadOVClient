<template>
    <div class="d-flex align-center title-bar">
        <v-btn small icon @click="navBack">
            <v-icon>
                mdi-chevron-left
            </v-icon>
        </v-btn>

        <v-btn small icon @click="navForward">
            <v-icon>
                mdi-chevron-right
            </v-icon>
        </v-btn>

        <v-btn small icon @click="navRefresh">
            <v-icon>
                mdi-refresh
            </v-icon>
        </v-btn>

        <div id="dragRegion">
            <v-img
                class="mx-2"
                :width="24"
                :height="24"
                :max-width="24"
                :max-height="24"
                :src="$root.generateAssetUri('assets/icon.png')"
                contain
            >
            </v-img>
        </div>

        <v-btn small icon class="mx-1" @click="minimize">
            <v-icon>
                mdi-window-minimize
            </v-icon>
        </v-btn>

        <v-btn small icon class="mx-1" @click="toggleMaximize">
            <v-icon>
                mdi-window-maximize
            </v-icon>
        </v-btn>

        <v-btn small icon class="mx-1" @click="closeWindow">
            <v-icon>
                mdi-window-close
            </v-icon>
        </v-btn>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'

/// #if DESKTOP
import { ipcRenderer } from 'electron'
/// #endif

@Component
export default class TitleBar extends Vue {
    navBack() {
        this.$router.back()
    }

    navForward() {
        this.$router.forward()
    }

    navRefresh() {
        this.$router.go(0)
    }

    minimize() {
        ipcRenderer.send('minimize')
    }

    toggleMaximize() {
        ipcRenderer.send('maximize')
    }

    closeWindow() {
        ipcRenderer.send('closeWindow')
    }
}

</script>

<style scoped>

#dragRegion {
    flex-grow: 1;
    -webkit-app-region: drag;
    height: 100%;
}

.title-bar {
    display: block;
    position: fixed;
    height: 32px;
    width: 100%;
    padding: 4px;
}

</style>