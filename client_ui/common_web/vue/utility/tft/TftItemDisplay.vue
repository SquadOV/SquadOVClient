<template>
    <v-img
        :src="itemImage"
        :width="widthHeight"
        :height="widthHeight"
        :max-width="widthHeight"
        :max-height="widthHeight"
    >
    </v-img>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { staticClient } from '@client/js/staticData'

@Component
export default class TftItemDisplay extends Vue {
    @Prop({required: true})
    item!: number

    @Prop({required: true})
    tftSet!: string

    @Prop({default: 32})
    widthHeight!: number

    get itemImage(): string {
        return staticClient.getTftItemIcon(this.tftSet, this.item)
    }
}

</script>