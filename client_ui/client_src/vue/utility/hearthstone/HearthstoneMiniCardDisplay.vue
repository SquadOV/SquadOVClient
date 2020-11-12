<template>
    <v-hover v-slot="{ hover }">
        <div :style="parentStyle">
            <div :style="divStyle">    
            </div>

            <hearthstone-full-card-display
                ref="hoverCard"
                v-if="hover"
                :card-id="cardId"
                :style="cardStyle"
            >
            </hearthstone-full-card-display>
        </div>
    </v-hover>
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

    hoverCardStyle: any = null

    $refs!: {
        hoverCard: HearthstoneFullCardDisplay
    }

    get maxWidth(): number {
        return this.maxHeight * 0.8
    }

    get cardSrc() : string {
        return staticClient.getHearthstoneCardPortraitUrl(this.cardId)
    }

    get parentStyle() : any {
        return {
            'position': 'relative',
            'top': '0',
            'left': '0',
        }
    }

    get divStyle() : any {
        return {
            'height': `${this.maxHeight}px`,
            'width': `${this.maxWidth}px`,
            'background-image': `url('${this.cardSrc}')`,
            'background-size': `${this.maxHeight*1.05}px ${this.maxHeight*1.05}px`,
            'background-position': '50% 50%',
            'border-radius': '50% 50%',
            'border': '2px solid white',
            'z-index': 1,
        }
    }

    recomputeHoverCardStyle() {
        if (!this.hoverCardStyle) {
            Vue.nextTick(() => {
                // Check if the hover card would go out of bounds on the right if displayed, if so
                // make it spawn to the left of the mini-card instead.
                let style : any = {
                    'position': 'absolute',
                    'top': 0,
                    'z-index': 2,
                }

                let bounding = this.$refs.hoverCard.$el.getBoundingClientRect()

                // Need to guesstimate bounding right as it won't be set at this point. We know the dimensions
                // of the card though so this should be fairly accurate.
                let rightBound = bounding.left + 362

                if (rightBound >= window.innerWidth) {
                    style['right'] =`${this.maxWidth * 1.01}px`
                } else {
                    style['left'] =`${this.maxWidth * 1.01}px`
                }

                this.hoverCardStyle = style
            })
        }
    }

    get cardStyle() : any {
        this.recomputeHoverCardStyle()
        return !!this.hoverCardStyle ? this.hoverCardStyle : {
            'position': 'absolute',
            'top': 0,
            'left': 0,
            'z-index': 2,
        }
    }
}

</script>