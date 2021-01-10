<template>
    <div v-if="!!heroCard">
        <v-img
            :src="imageUrl"
            :max-height="maxHeight"
            :max-width="maxHeight"
            contain
        >
        </v-img>
    </div>
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

    get imageUrl() : string {
        return staticClient.getHearthstoneCardPortraitUrl(this.heroCard!)
    }
}

</script>