<template>
    <div id="rootApp">
        <keep-alive :max="3">
            <router-view :key="key"></router-view>
        </keep-alive>
        <!-- Manual URL display dialog -->
        <v-dialog v-model="showUrl" max-width="40%">
            <v-card>
                <v-card-title>
                    We're opening up a URL in your browser!
                </v-card-title>
                <v-divider></v-divider>

                <div class="ma-4">
                    Just in case that didn't work, here's a URL that you can copy and paste into a browser of your choosing.

                    <v-text-field
                        class="mt-4"
                        :value="$store.state.redirectUrl"
                        hide-details
                        single-line
                        outlined
                        dense
                        readonly
                        ref="urlInput"
                    >
                        <template v-slot:append-outer>
                            <v-btn
                                icon
                                color="success"
                                @click="doCopy"
                            >
                                <v-icon>
                                    mdi-content-copy
                                </v-icon>
                            </v-btn>
                        </template>
                    </v-text-field>
                </div>

                <v-snackbar
                    v-model="showUrlCopy"
                    :timeout="5000"
                    color="success"
                >
                    Copied URL to clipboard!
                </v-snackbar>
            </v-card>
        </v-dialog>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import { routeLevelToKey } from '@client/js/routes'

@Component
export default class App extends Vue {
    showUrl: boolean = false
    showUrlCopy: boolean = false
    closeUrlTimeout: number | null = null

    $refs!: {
        urlInput: any
    }

    @Watch('$store.state.redirectUrl')
    onUrlChange(url: string | null) {
        if (!url) {
            return
        }

        if (this.closeUrlTimeout !== null) {
            clearTimeout(this.closeUrlTimeout)
        }

        this.showUrl = true
        this.closeUrlTimeout = window.setTimeout(() => {
            this.closeUrlTimeout = null
            this.showUrl = false
        }, 3000)
    }

    doCopy() {
        let inputEle = this.$refs.urlInput.$el.querySelector('input')
        inputEle.select()
        document.execCommand('copy')
        this.showUrlCopy = true
    }

    get key(): string {
        // We need to use a custom key here. We only want
        // to re-render this router view when the top level route
        // changes - not when a child level route changes.
        return routeLevelToKey(this.$route, 1)
    }
}

</script>

<style scoped>

#rootApp {
    width: 100%;
    height: 100%;
    overflow: auto;
}

</style>