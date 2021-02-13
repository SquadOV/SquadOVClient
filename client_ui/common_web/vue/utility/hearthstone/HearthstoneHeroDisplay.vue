<template>
    <v-img
        v-if="!!heroCard"
        :src="imageUrl"
        :max-height="cssMaxHeight"
        :max-width="maxHeight"
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
export default class HearthstoneHeroDisplay extends Vue {
    @Prop({required: true})
    heroCard!: string | undefined

    @Prop({default: 100})
    maxHeight!: number

    @Prop({type: Boolean, default: false})
    fill!: boolean

    get cssMaxHeight(): string {
        return this.fill ? '100%' : `${this.maxHeight}px`
    }

    get imageUrl() : string {
        return staticClient.getHearthstoneCardPortraitUrl(this.heroCard!)
    }
}

</script>