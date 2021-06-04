<template>
    <v-tooltip
        :left="left"
        :right="!left"
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

    @Prop({type: Boolean, default: false})
    history!: boolean

    @Prop({type: Boolean, default: false})
    isFriendly!: boolean

    get maxWidth(): number {
        return this.maxHeight * 0.8
    }

    get cardSrc() : string {
        return staticClient.getHearthstoneCardPortraitUrl(this.cardId)
    }

    get divStyle() : any {
        let style : any = {
            'height': `${this.maxHeight}px`,
            'background-image': `url('${this.cardSrc}')`,
            'background-size': `${this.maxHeight*1.2}px ${this.maxHeight*1.2}px`,
            'background-position': '55% 40%',
            'z-index': 1,
        }

        if (this.history) {
            let histColor = this.isFriendly ? 'color-friendly' : 'color-enemy'
            style['border-radius'] = '4px'
            style['border'] = `4px solid var(--${histColor})`
            style['width'] = `${this.maxHeight}px`
        } else {
            style['border-radius'] = '50%'
            style['border'] = '2px solid white'
            style['width'] = `${this.maxWidth}px`
        }

        if (this.noShrink) {
            style['flex-shrink'] = '0 !important'
        }

        return style
    }
}

</script>