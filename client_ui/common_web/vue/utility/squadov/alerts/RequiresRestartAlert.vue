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
                    Certain changes to your SquadOV settings requires a restart before taking effect.
                </div>
            </div>
        </v-alert>
    </div>
</template>

<script lang="ts">
import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'

@Component
export default class RequiresRestartAlert extends Vue {
    showAlert: boolean = false
    onClose() {
        this.muteAlert()
    }

    muteAlert() {
        window.sessionStorage.setItem('muteRequiresRestart', 'true')
    }

    get isAlarmMute() {
        return (window.sessionStorage.getItem('muteRequiresRestart') == 'true' ? true : false)
    }

    @Watch('$store.state.requiresRestart')
    resync() {
        if (!this.isAlarmMute) {
            this.showAlert = this.$store.state.requiresRestart
        }
    }

    mounted() {
        this.resync()
    }
}
</script>
