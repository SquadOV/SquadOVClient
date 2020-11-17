<template>
    <v-tooltip
        left
        :allow-overflow="!noOverflow"
        color="transparent"
    >
        <template v-slot:activator="{on, attrs}">
            <div
                :style="divStyle"
                v-bind="attrs"
                v-on="on"
            >
            </div>
        </template>
        <hearthstone-full-card-display
            :card-id="cardId"
        >
        </hearthstone-full-card-display>
    </v-tooltip>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { staticClient } from '@client/js/staticData'
import HearthstoneFullCardDisplay from '@client/vue/utility/hearthstone/HearthstoneFullCardDisplay.vue'

@Component({
    components: {
        HearthstoneFullCardDisplay
    }
})
export default class HearthstoneMiniCardDisplay extends Vue {
    @Prop({required: true})
    cardId!: string

    @Prop({default: 130})
    maxHeight!: number

    @Prop({type: Boolean, default: false})
    left!: boolean

    @Prop({type: Boolean, default: false})
    noShrink!: boolean

    @Prop({type: Boolean, default: false})
    noOverflow!: boolean

    get maxWidth(): number {
        return this.maxHeight * 0.8
    }

    get cardSrc() : string {
        return staticClient.getHearthstoneCardPortraitUrl(this.cardId)
    }

    get divStyle() : any {
        let style : any = {
            'height': `${this.maxHeight}px`,
            'width': `${this.maxWidth}px`,
            'background-image': `url('${this.cardSrc}')`,
            'background-size': `${this.maxHeight*1.05}px ${this.maxHeight*1.05}px`,
            'background-position': '50% 50%',
            'border-radius': '50% 50%',
            'border': '2px solid white',
            'z-index': 1,
        }

        if (this.noShrink) {
            style['flex-shrink'] = '0 !important'
        }

        return style
    }
}

</script>