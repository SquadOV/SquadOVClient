<template>
    <v-btn :href="downloadHref" target="_blank" @click="onDownload" color="success" :large="large">
        Download SquadOV
    </v-btn>
</template>

<script lang="ts">

import CommonComponent from '@client/vue/CommonComponent'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { apiClient } from '@client/js/api'
import { AnalyticsAction, AnalyticsCategory } from '@client/js/analytics/events'

@Component
export default class DownloadButton extends CommonComponent {
    @Prop({type: Boolean, default: false})
    large!: boolean

    @Prop({type: Boolean, default: false})
    setupWizard!: boolean

    get downloadHref(): string {
        return 'https://us-central1.content.squadov.gg/builds/SquadOV.exe'
    }

    onDownload() {
        if (!localStorage.getItem('squadovDownload')) {
            apiClient.markUserDownload().then(() => {
                localStorage.setItem('squadovDownload', 'yes')
            }).catch((err: any) => {
                console.log('Failed to mark user download: ', err)
            })
        }

        if (this.setupWizard) {
            this.analytics?.event(this.$route, AnalyticsCategory.SetupWizard, AnalyticsAction.SetupDoDownload, '', 0)
        }
    }
}

</script>