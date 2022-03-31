<template>
    <loading-container :is-loading="!hasSettings">
        <template v-slot:default="{ loading }">
            <v-container fluid v-if="!loading">
                <div class="d-flex align-center">
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

    @Watch('bookmarkRecord', {deep: true})
    @Watch('$store.state.settings.keybinds.bookmark')
    refreshBookmarkKeybindStrings() {
        KbManager.keybindToString(!!this.bookmarkRecord ? this.bookmarkRecord.keybind : this.$store.state.settings.keybinds.bookmark).then((resp: string) => {
            this.bookmarkKeybindStr = resp
        })
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
    }
}

</script>