<template>
    <v-btn :to="to" target="_blank" color="success" :large="large">
        Download SquadOV
    </v-btn>
</template>

<script lang="ts">

import CommonComponent from '@client/vue/CommonComponent'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { DownloadThanksPageId } from '@client/js/pages'

@Component
export default class DownloadButton extends CommonComponent {
    @Prop({type: Boolean, default: false})
    large!: boolean

    get to(): any {
        return {
            name: DownloadThanksPageId,
        }
    }
}

</script>