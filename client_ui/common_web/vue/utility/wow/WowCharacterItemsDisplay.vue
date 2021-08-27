<template>
    <div class="d-flex flex-wrap justify-center full-width">
        <div
            class="d-flex justify-space-between align-center my-2 mr-8"
            v-for="(item, idx) in items"
            :key="idx"
            style="width: calc(50% - 32px)"
        >
            <div class="d-flex justify-center full-width text-subtitle-2 font-weight-bold" v-if="item.itemId === 0">
                No Item Equipped.
            </div>

            <template v-else-if="!!data[item.itemId]">
                <div class="text-subtitle-2 font-weight-bold mr-2" style="width: 75px; min-width: 75px; max-width: 75px;">
                    {{ wowItemSlotToName(data[item.itemId].inventorySlot)}}
                </div>

                <v-divider vertical></v-divider>

                <wow-single-item-display
                    class="mx-2"
                    :item="item"
                    :patch="patch"
                >
                </wow-single-item-display>

                <v-divider vertical></v-divider>

                <div class="text-subtitle-2 font-weight-bold ml-2" style="width: 25px; min-width: 25px; max-width: 25px;">
                    {{ item.ilvl }}
                </div>
            </template>

            <v-progress-circular size="16" indeterminate v-else></v-progress-circular>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { WowItem, wowItemSlotToName } from '@client/js/wow/character'
import { wowCache, WowItemStatic } from '@client/js/wow/staticCache'
import WowSingleItemDisplay from '@client/vue/utility/wow/WowSingleItemDisplay.vue'

@Component({
    components: {
        WowSingleItemDisplay
    }
})
export default class WowCharacterItemsDisplay extends Vue {
    wowItemSlotToName = wowItemSlotToName

    @Prop({type: Array, required: true})
    items!: WowItem[]

    @Prop({required: true})
    patch!: string

    data: { [item: number] : WowItemStatic | undefined} = {}

    @Watch('items')
    refreshCache() {
        wowCache.getCache(this.patch).bulkGetItems(this.items.map((ele: WowItem) => ele.itemId)).then((resp: Map<number, WowItemStatic>) => {
            for (let [key, value] of resp) {
                Vue.set(this.data, key, value)
            }
        }).catch((err: any) => {
            console.error('Failed to get items: ', err)
        })
    }

    mounted() {
        this.refreshCache()
    }
}

</script>