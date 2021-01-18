<template>
    <v-img
        v-if="hasLane"
        :src="$root.generateAssetUri(src)"
        :width="widthHeight"
        :height="widthHeight"
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

@Component
export default class LolLaneIcon extends Vue {
    @Prop({required: true})
    lane!: string

    @Prop({default: 16})
    widthHeight!: number

    get hasLane(): boolean {
        return this.src.length > 0
    }

    get src(): string {
        switch (this.lane.toUpperCase()) {
            case 'MID':
            case 'MIDDLE':
                return 'assets/lol/lane/middle.png'
            case 'TOP':
                return 'assets/lol/lane/top.png'
            case 'JUNGLE':
                return 'assets/lol/lane/jungle.png'
            case 'BOT':
            case 'BOTTOM':
                return 'assets/lol/lane/bottom.png'
        }
        return ''
    }
}

</script>