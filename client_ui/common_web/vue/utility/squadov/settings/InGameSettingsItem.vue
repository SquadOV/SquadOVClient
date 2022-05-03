<template>
    <loading-container :is-loading="!hasSettings">
        <template v-slot:default="{ loading }">
            <v-container fluid v-if="!loading">
                <div class="d-flex align-center">
                    <keybind-settings-item
                        label="Clip"
                        :value="$store.state.settings.keybinds2.clip2"
                        @input="$store.commit('changeClipKeybind', arguments[0])"
                        allow-hold
                    >
                    </keybind-settings-item>

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

                                    Creates a clip with a length of this many seconds when you activate the hotkey.
                                    This is currently only available for users who are not locally recording.
                                </v-tooltip>
                            </div>
                        </template>
                    </v-text-field>
                </div>

                <div class="d-flex align-center mt-4">
                    <keybind-settings-item
                        label="Bookmark"
                        :value="$store.state.settings.keybinds2.bookmark"
                        @input="$store.commit('changeBookmarkKeybind', arguments[0])"
                        allow-hold
                    >
                    </keybind-settings-item>
                </div>
            </v-container>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import KeybindSettingsItem from '@client/vue/utility/squadov/settings/KeybindSettingsItem.vue'

@Component({
    components: {
        LoadingContainer,
        KeybindSettingsItem,
    }
})
export default class InGameSettingsItem extends Vue {
    get hasSettings(): boolean {
        return !!this.$store.state.settings
    }
}

</script>