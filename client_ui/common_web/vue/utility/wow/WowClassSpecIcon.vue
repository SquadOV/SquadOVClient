<template>
    <v-img
        :src="iconUrl"
        :max-width="widthHeight"
        :max-height="widthHeight"
        contain
    >
    </v-img>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { staticClient } from '@client/js/staticData'

@Component
export default class WowClassSpecIcon extends Vue {
    @Prop({type: Number})
    specId!: number

    @Prop({type: Number, default: 32})
    widthHeight!: number

    @Prop({required: true})
    patch!: string

    get iconUrl(): string {
        return staticClient.getWowSpecsIconUrl(this.patch, this.specId)
    }
}

</script>