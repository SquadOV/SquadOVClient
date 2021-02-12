<template>
    <v-img
        :src="cardSrc"
        :height="maxHeight"
        :width="width"
        :style="style"
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
export default class HearthstoneFullCardDisplay extends Vue {
    @Prop({required: true})
    cardId!: string

    @Prop({default: 500})
    maxHeight!: number

    get cardSrc() : string {
        return staticClient.getHearthstoneCardImageUrl(this.cardId)
    }

    get width() : number {
        // Specify width manually to allow the tooltip to be positioned properly before the image loads.
        return this.maxHeight * 0.723938
    }

    get style() : any {
        return {
            'max-width': 'none !important'
        }
    }
}

</script>