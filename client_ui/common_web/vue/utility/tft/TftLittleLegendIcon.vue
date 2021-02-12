<template>
    <v-img
        :src="imgUrl"
        :width="width"
        :height="height"
        :max-width="width"
        :max-height="height"
    >
    </v-img>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { staticClient } from '@client/js/staticData'

@Component
export default class TftLittleLegendIcon extends Vue {
    @Prop({required: true})
    contentId!: string

    @Prop({default: 128})
    width!: number

    @Prop({default: 128})
    height!: number

    get imgUrl(): string {
        return staticClient.getTftLittleLegendIcon(this.contentId)
    }
}

</script>