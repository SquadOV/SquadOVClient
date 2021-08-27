<template>
    <wow-single-item-display
        v-if="conduitId != 0 && baseItemId !== null"
        :width-height="widthHeight"
        :item="item"
        tooltip-mode
        :patch="patch"
    >
    </wow-single-item-display>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { staticClient } from '@client/js/staticData'
import { wowCache } from '@client/js/wow/staticCache'
import WowSingleItemDisplay from '@client/vue/utility/wow/WowSingleItemDisplay.vue'
import { WowItem } from '@client/js/wow/character'

@Component({
    components: {
        WowSingleItemDisplay
    }
})
export default class WowConduitIcon extends Vue {
    @Prop({required: true})
    conduitId!: number

    @Prop({required: true})
    ilvl!: number

    @Prop({type: Number, default: 32})
    widthHeight!: number

    @Prop({required: true})
    patch!: string

    baseItemId: number | null = null

    get src(): string {
        return staticClient.getWowSpellIconUrl(this.patch, this.conduitId)
    }

    get item(): WowItem {
        return {
            itemId: (this.baseItemId != null) ? this.baseItemId : 6603,
            ilvl: this.ilvl,
        }
    }

    @Watch('conduitId')
    refreshItem() {
        wowCache.getCache(this.patch).getConduitItem(this.conduitId).then((resp: number) => {
            this.baseItemId = resp
        }).catch((err: any) => {
            console.warn('Failed to get conduit: ', err)
        })
    }

    mounted() {
        this.refreshItem()
    }
}

</script>