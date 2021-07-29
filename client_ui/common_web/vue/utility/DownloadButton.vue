<template>
    <v-btn :href="downloadHref" target="_blank" @click="onDownload" color="success" :large="large">
        Download SquadOV
    </v-btn>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { apiClient } from '@client/js/api'

@Component
export default class DownloadButton extends Vue {
    @Prop({type: Boolean, default: false})
    large!: boolean

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
    }
}

</script>