<template>
    <loading-container :is-loading="!hasSettings">
        <template v-slot:default="{ loading }">
            <v-container fluid v-if="!loading">
                <div class="d-flex align-center">
                    <div>
                        Clip: 
                    </div>

                    <v-text-field
                        :value="clipKeybindStr"
                        class="ml-8 flex-grow-0"
                        solo
                        single-line
                        hide-details
                        readonly
                        style="width: 500px;"
                    >
                        <template v-slot:append>
                            <v-btn class="primary" v-if="!clipRecord" @click="startClipRecord">
                                Edit Keybind
                            </v-btn>

                            <v-btn class="error" @click="stopClipRecord" v-else>
                                Stop Recording
                            </v-btn>
                        </template>
                    </v-text-field>

                    <div class="ml-4">
                        Length (seconds):
                    </div>

                    <v-text-field
                        :value="$store.state.settings.instantClipLengthSeconds"
                        @change="$store.commit('changeInstantClipLength', parseInt(arguments[0]))"
                        type="number"
                        solo
                        hide-details
                        class="ml-8"
                    >
                        <template v-slot:append>
                            <div class="d-flex align-center">
                                <v-tooltip bottom max-width="450px">
                                    <template v-slot:activator="{on, attrs}">
                                        <v-icon v-on="on" v-bind="attrs">
                                            mdi-help-circle
                                        </v-icon>
                                    </template>

                                    Creates a clip with a length of this many seconds when you click the {{ clipKeybindStr }} button.
                                    This is currently only available for users who are not locally recording.
                                </v-tooltip>
                            </div>
                        </template>
                    </v-text-field>
                </div>

                <div class="d-flex align-center mt-4">
                    <div>
                        Bookmark: 
                    </div>

                    <v-text-field
                        :value="bookmarkKeybindStr"
                        class="ml-8 flex-grow-0"
                        solo
                        single-line
                        hide-details
                        readonly
                        style="width: 500px;"
                    >
                        <template v-slot:append>
                            <v-btn class="primary" v-if="!bookmarkRecord" @click="startBookmarkRecord">
                                Edit Keybind
                            </v-btn>

                            <v-btn class="error" @click="stopBookmarkRecord" v-else>
                                Stop Recording
                            </v-btn>
                        </template>
                    </v-text-field>
                </div>
            </v-container>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import KbManager, { KeybindRecordingSession } from '@client/js/system/keybinds'

@Component({
    components: {
        LoadingContainer
    }
})
export default class InGameSettingsItem extends Vue {
    KbManager = KbManager

    get hasSettings(): boolean {
        return !!this.$store.state.settings
    }

    bookmarkRecord: KeybindRecordingSession | null = null
    bookmarkKeybindStr: string = ''

    clipRecord: KeybindRecordingSession | null = null
    clipKeybindStr: string = ''

    @Watch('bookmarkRecord', {deep: true})
    @Watch('$store.state.settings.keybinds.bookmark')
    refreshBookmarkKeybindStrings() {
        KbManager.keybindToString(!!this.bookmarkRecord ? this.bookmarkRecord.keybind : this.$store.state.settings.keybinds.bookmark).then((resp: string) => {
            this.bookmarkKeybindStr = resp
        })
    }

    @Watch('clipRecord', {deep: true})
    @Watch('$store.state.settings.keybinds.clip')
    refreshClipKeybindStrings() {
        KbManager.keybindToString(!!this.clipRecord ? this.clipRecord.keybind : this.$store.state.settings.keybinds.clip).then((resp: string) => {
            this.clipKeybindStr = resp
        })
    }

    startClipRecord() {
        if (!!this.clipRecord) {
            return
        }

        this.$store.commit('changeClipKeybind', [])
        this.clipRecord = Vue.observable(new KeybindRecordingSession())
    }

    stopClipRecord() {
        if (!this.clipRecord) {
            return
        }

        this.$store.commit('changeClipKeybind', this.clipRecord.keybind)
        this.clipRecord.close()
        this.clipRecord = null
    }

    startBookmarkRecord() {
        if (!!this.bookmarkRecord) {
            return
        }

        this.$store.commit('changeBookmarkKeybind', [])
        this.bookmarkRecord = Vue.observable(new KeybindRecordingSession())
    }

    stopBookmarkRecord() {
        if (!this.bookmarkRecord) {
            return
        }

        this.$store.commit('changeBookmarkKeybind', this.bookmarkRecord.keybind)
        this.bookmarkRecord.close()
        this.bookmarkRecord = null
    }

    mounted() {
        this.refreshBookmarkKeybindStrings()
        this.refreshClipKeybindStrings()
    }
}

</script>