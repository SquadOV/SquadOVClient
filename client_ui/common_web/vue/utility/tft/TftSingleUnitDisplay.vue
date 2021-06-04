<template>
    <div class="top-div">
        <v-tooltip top>
            <template v-slot:activator="{on, attrs}">
                <div
                    class="d-flex flex-column justify-start align-center"
                    v-on="on"
                    v-bind="attrs"
                >
                    <!-- Tier -->
                    <img class="star-image" :src="starImageUrl"/>

                    <!-- Unit -->
                    <v-img
                        class="my-1"
                        :src="unitImageUrl"
                        :width="width"
                        :height="width"
                        :max-width="width"
                        :max-height="width"
                        contain
                        :style="unitStyle"
                    >
                    </v-img>

                    <img class="chosen-style" v-if="!!unit.chosen" :src="chosenImage"/>

                    <!-- Items -->
                    <div class="d-flex flex-wrap justify-center align-center">
                        <tft-item-display
                            v-for="(item, idx) in unit.items"
                            :key="idx"
                            :item="item"
                            :tft-set="tftSet"
                            :width-height="width / 3"
                        >
                        </tft-item-display>
                    </div>
                </div>
            </template>

            <div class="d-flex flex-column" v-if="!!unitData">
                <!-- Unit Name + Star Level -->
                <div class="d-flex align-center">
                    <div class="text-h6 font-weight-bold mr-1">
                        {{ unitData.name }}
                    </div>
                    <img class="star-image" :src="starImageUrl"/>
                </div>

                <!-- Unit Traits + Chosen -->
                <div class="d-flex flex-wrap">
                    <img class="mr-1" v-if="!!unit.chosen" :src="chosenImage"/>
                    <template v-for="(t, idx) in traitData">
                        <span
                            :key="`trait-${idx}`"
                            :class="`${unit.chosen === t.key ? 'chosen' : ''}`"
                        >
                            {{ t.name }}
                        </span>

                        <span
                            class="mx-1"
                            :key="`separator-${idx}`"
                            v-if="idx < traitData.length - 1"
                        >
                            &middot;
                        </span>
                    </template>
                </div>

                <!-- Unit Items with Names -->
                <div
                    class="d-flex my-1 align-center"
                    v-for="(item, idx) in itemData"
                    :key="`item-${idx}`"
                >
                    <tft-item-display
                        class="mr-1"
                        :item="item.id"
                        :tft-set="tftSet"
                        :width-height="32"
                    >
                    </tft-item-display>

                    <div class="text-subtitle-2">
                        {{ item.name }}
                    </div>
                </div>
            </div>
        </v-tooltip>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { TftUnit, TftUnitData } from '@client/js/tft/units'
import { TftTraitData } from '@client/js/tft/traits'
import { TftItemData } from '@client/js/tft/items'
import { staticClient } from '@client/js/staticData'
import { ApiData } from '@client/js/api'
import {
    getTftCostColor
} from '@client/js/tft/colors'
import TftItemDisplay from '@client/vue/utility/tft/TftItemDisplay.vue'
import axios from 'axios'

@Component({
    components: {
        TftItemDisplay
    }
})
export default class TftSingleUnitDisplay extends Vue {
    @Prop({required: true})
    unit!: TftUnit

    @Prop({required: true})
    tftSet!: string

    @Prop({default: 64})
    width!: number

    unitData: TftUnitData | null = null
    traitData: TftTraitData[] = []
    itemData: TftItemData[] = []

    get starImageUrl(): string {
        //@ts-ignore
        return this.$root.generateAssetUri(`assets/tft/units/${this.unit.tier}_star_trim.png`)
    }

    get unitImageUrl(): string {
        if (!this.unit.characterId) {
            return ''
        }
        return staticClient.getTftUnitIcon(this.tftSet, this.unit.characterId)
    }

    refreshData() {
        this.refreshUnitData()
        this.refreshItemData()
    }

    refreshUnitData() {
        this.unitData = null
        if (!this.unit.characterId) {
            return
        }

        axios.get(staticClient.getTftUnitData(this.tftSet, this.unit.characterId)).then((resp: ApiData<TftUnitData>) => {
            this.unitData = resp.data
            this.refreshTraitData()
        }).catch((err: any) => {
            console.log('Failed to get TFT unit data: ', err)
        })
    }

    refreshTraitData() {
        this.traitData = []
        if (!this.unitData) {
            return
        }

        if (this.tftSet.startsWith('3')) {
            this.traitData = this.unitData.traits.map((ele: string) => {
                return {
                    key: ele,
                    name: ele
                }
            })
        } else {
            Promise.all(this.unitData.traits.map((ele: string) => {
                return axios.get(staticClient.getTftTraitData(this.tftSet, ele))
            })).then((values: ApiData<TftTraitData>[]) => {
                this.traitData = values.map((ele: ApiData<TftTraitData>) => ele.data)
            }).catch((err: any) => {
                console.log('Failed to get TFT unit trait data: ', err)
            })
        }
    }

    refreshItemData() {
        this.itemData = []
        if (this.unit.items.length == 0) {
            return
        }

        Promise.all(this.unit.items.map((ele: number) => {
            return axios.get(staticClient.getTftItemData(this.tftSet, ele))
        })).then((values: ApiData<TftItemData>[]) => {
            this.itemData = values.map((ele: ApiData<TftItemData>) => ele.data)
        }).catch((err: any) => {
            console.log('Failed to get TFT unit item data: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }

    get chosenImage(): string {
        //@ts-ignore
        return this.$root.generateAssetUri('assets/tft/traits/chosen_clipped.png')
    }

    get unitStyle() : any {
        return {
            'border-radius': '5px',
            'border': `2px solid var(--${getTftCostColor(!!this.unitData ? this.unitData.cost : 1)})`
        }
    }
}

</script>

<style scoped>

.star-image {
    display: block;
    height: 14px;
}

.chosen {
    color: #FFC0CB;
}

.top-div {
    position: relative;
}

.chosen-style {
    position: absolute;
    top: 9px;
    right: -9px;
}

</style>