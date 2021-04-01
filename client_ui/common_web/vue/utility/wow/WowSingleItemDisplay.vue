<template>
    <div class="d-flex align-center">
        <v-tooltip bottom :disabled="!tooltipMode">
            <template v-slot:activator="{on, attrs}">
                <v-img
                    v-bind="attrs"
                    v-on="on"
                    :class="tooltipMode ? '': 'mx-2'"
                    :width="widthHeight"
                    :height="widthHeight"
                    :max-width="widthHeight"
                    :max-height="widthHeight"
                    contain
                    :src="staticClient.getWowItemIconUrl(item.itemId)"
                    :style="itemImageStyling(data[item.itemId])"
                >
                </v-img>
            </template>

            <div v-if="!!data[item.itemId]">
                {{ data[item.itemId].name }} ({{ item.ilvl }})
            </div>
        </v-tooltip>

        <div class="text-subtitle-2 font-weight-bold" v-if="!!data[item.itemId] && !tooltipMode" :style="itemTextStyling(data[item.itemId])">
            {{ data[item.itemId].name }}
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { WowItem } from '@client/js/wow/character'
import { staticClient } from '@client/js/staticData'
import { wowItemQualityToColor } from '@client/js/wow/colors'
import { colorToCssString } from '@client/js/color'
import { wowCache, WowItemStatic } from '@client/js/wow/staticCache'

@Component
export default class WowSingleItemDisplay extends Vue {
    staticClient = staticClient

    @Prop({required: true})
    item!: WowItem

    @Prop({type: Number, default: 32})
    widthHeight!: number

    @Prop({type: Boolean, default: false})
    tooltipMode!: boolean

    @Prop({required: true})
    patch!: string

    data: { [item: number] : WowItemStatic | undefined} = {}

    @Watch('item')
    refreshCache() {
        wowCache.getCache(this.patch).bulkGetItems([this.item.itemId]).then((resp: Map<number, WowItemStatic>) => {
            for (let [key, value] of resp) {
                Vue.set(this.data, key, value)
            }
        }).catch((err: any) => {
            console.log('Failed to get item: ', err)
        })
    }

    mounted() {
        this.refreshCache()
    }

    
    itemImageStyling(item: WowItemStatic | undefined): any {
        if (!item) {
            return {}
        }

        return {
            border: `2px solid ${colorToCssString(wowItemQualityToColor(item.quality))}`
        }
    }

    itemTextStyling(item: WowItemStatic | undefined): any {
        if (!item) {
            return {}
        }

        return {
            color: `${colorToCssString(wowItemQualityToColor(item.quality))}`
        }
    }
}

</script>