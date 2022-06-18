<template>
    <div>
        <v-alert
            v-if="showAlert"
            class="pa-2 ma-0"
            type="info"
            dismissible
            tile
            width="100%"
            @input="onClose"
        >
            <template v-slot:prepend>
                <v-icon>mdi-exclamation</v-icon>
            </template>

            <div class="d-flex align-center">
                <div>
                    You're on a Pro plan with watermarks enabled. Don't forget you can turn it off!
                </div>

                <v-spacer></v-spacer>

                <v-btn
                    small
                    class="ml-2"
                    color="secondary"
                    :to="settingsTo"
                >
                    Settings
                </v-btn>
            </div>
        </v-alert>
    </div>
</template>

<script lang="ts">
import { SettingsPageId } from '@client/js/pages'
import { EPricingTier } from '@client/js/squadov/pricing'
import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'

@Component
export default class ProWatermarkAlert extends Vue {
    showAlert: boolean = false

    get settingsTo(): any {
        return {
            name: SettingsPageId,
            query: {
                inputTab: 3,
            },
        }
    }

    onClose() {
        this.muteAlert()
    }

    muteAlert() {
        window.localStorage.setItem('muteProWatermark', 'true')
    }

    get isAlarmMute() {
        return (window.localStorage.getItem('muteProWatermark') == 'true' ? true : false)
    }

    @Watch('$store.state.settings.record.watermark.enabled')
    @Watch('$store.state.tier')
    resync() {
        if (!this.isAlarmMute) {
            this.showAlert = !!this.$store.state.tier && (this.$store.state.tier != EPricingTier.Basic) && !!this.$store.state.settings.record.watermark.enabled
        }
    }

    mounted() {
        this.resync()
    }
}
</script>
