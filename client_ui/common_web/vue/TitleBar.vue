<template>
    <div class="d-flex align-center title-bar">
        <v-btn small icon @click="navBack" v-if="!noNav">
            <v-icon>
                mdi-chevron-left
            </v-icon>
        </v-btn>

        <v-btn small icon @click="navForward" v-if="!noNav">
            <v-icon>
                mdi-chevron-right
            </v-icon>
        </v-btn>

        <v-btn small icon @click="navRefresh" v-if="!noNav">
            <v-icon>
                mdi-refresh
            </v-icon>
        </v-btn>

        <div id="dragRegion">
            <v-img
                v-if="!noNav"
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

        <v-btn small icon class="mx-1" @click="minimize" v-if="!noNav">
            <v-icon>
                mdi-window-minimize
            </v-icon>
        </v-btn>

        <v-btn small icon class="mx-1" @click="toggleMaximize" v-if="!noNav">
            <v-icon>
                mdi-window-maximize
            </v-icon>
        </v-btn>

        <v-btn small icon class="mx-1" @click="closeWindow" v-if="!noNav">
            <v-icon>
                mdi-window-close
            </v-icon>
        </v-btn>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'

/// #if DESKTOP
import { ipcRenderer } from 'electron'
/// #endif

@Component
export default class TitleBar extends Vue {
    @Prop({type: Boolean, default: false})
    noNav!: boolean

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

    keydownHandler: any = null

    handleKeypress(e: KeyboardEvent) {
        let cmp = e.key.toLowerCase()
        if (e.ctrlKey && cmp == 'r') {
            this.navRefresh()
        } else if (e.altKey && cmp == 'arrowleft') {
            this.navBack()
        } else if (e.altKey && cmp == 'arrowright') {
            this.navForward()
        }
    }

    mounted() {
        this.keydownHandler = this.handleKeypress.bind(this)
        window.addEventListener('keydown', this.keydownHandler)
    }

    beforeDestroy() {
        window.removeEventListener('keydown', this.keydownHandler)
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
    border-bottom: 1px solid #606060;
}

</style>