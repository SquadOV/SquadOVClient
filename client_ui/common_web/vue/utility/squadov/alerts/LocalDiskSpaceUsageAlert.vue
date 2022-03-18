<template>
    <v-alert
        v-if="showStorageWarning"
        class="pa-2 ma-0"
        type="warning"
        dismissible
        tile
        height="localDiskSpaceUsageAlertHeight"
        width="100%"
        @input="muteAlert"
    >
        <template v-slot:prepend>
            <v-icon>mdi-exclamation</v-icon>
        </template>
        <div class="d-flex align-center">
            <div>
                Local Storage location is running low. You only have
                {{ Math.round(Math.min(storageGBLeft, diskSpaceGBLeft) * 100) / 100 }}GB left. Either
                delete existing videos from Library > Local, permit SquadOV to
                use more space, or clean up space on your hard drive.
            </div>

            <v-spacer></v-spacer>

            <v-btn small class="ml-2" color="primary" :to="localStoragePageTo">
                Manage Storage
            </v-btn>
            <v-btn small class="ml-2" color="secondary" :to="accountSettingsTo">
                Go To Settings
            </v-btn>
        </div>
    </v-alert>
</template>

<script lang="ts">
import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import checkDiskSpace from 'check-disk-space'
import * as pi from '@client/js/pages'

@Component
export default class LocalDiskSpaceUsageAlert extends Vue {
    localDiskSpaceUsageAlertHeight: number = 45
    showStorageWarning: boolean = false
    showDiskSpaceWarning: boolean = false
    storageGBLeft: number = 0
    diskSpaceGBLeft: number = 999999999

    get maxLocalRecordingSizeGb(): number {
        return this.$store.state.settings.record.maxLocalRecordingSizeGb
    }

    get localDiskSpaceRecordUsageGb(): number | null {
///#if DESKTOP
        return this.$store.state.localDiskSpaceRecordUsageGb
///#else
        return null
///#endif
    }

    get isAlarmMute() {
        return (window.sessionStorage.getItem('muteLowStorageAlert') == 'true' ? true : false)
    }

    get localStoragePageTo(): any {
        return {
            name: pi.LocalStoragePageId,
        }
    }

    get accountSettingsTo(): any {
        return {
            name: pi.SettingsPageId,
            query: {
                inputTab: 5,
            },
        }
    }

    @Watch('localDiskSpaceRecordUsageGb')
    recheckLocalDiskSpaceRecordUsage() {
        if (this.localDiskSpaceRecordUsageGb === null) {
            return
        }

        this.storageGBLeft = this.maxLocalRecordingSizeGb - this.localDiskSpaceRecordUsageGb
        let storagePercentLeft = 100 - (this.localDiskSpaceRecordUsageGb / this.maxLocalRecordingSizeGb * 100)
        if ((this.storageGBLeft < 10 || storagePercentLeft < 10) && !this.isAlarmMute) {
            this.showStorageWarning = true
        }

        checkDiskSpace(this.$store.state.settings.record.localRecordingLocation).then((ds: any) => {
            this.diskSpaceGBLeft = ds.free / 1024.0 / 1024.0 / 1024.0
            if (this.diskSpaceGBLeft < 10 && !this.isAlarmMute) {
                this.showStorageWarning = true
            }
        })
    }

    muteAlert() {
        window.sessionStorage.setItem('muteLowStorageAlert', 'true')
    }
}
</script>