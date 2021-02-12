<template>
    <div>
        <v-tooltip bottom>
            <template v-slot:activator="{on, attrs}">
                <div
                    class="d-flex justify-center align-center"
                    :style="parentStyle"
                    v-on="on"
                    v-bind="attrs"
                >
                    <div :style="traitStyle">
                    </div>
                    <div :style="backgroundStyle">
                    </div>
                </div>
            </template>

            <span v-if="!!currentTrait">
                {{ trait.numUnits }}&nbsp;{{ currentTrait.name }}
            </span>
        </v-tooltip>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { staticClient } from '@client/js/staticData'
import { TftTraitData, TftTrait } from '@client/js/tft/traits'
import { getTftSetNumber } from '@client/js/tft/matches'
import { ApiData } from '@client/js/api'
import axios from 'axios'

@Component
export default class TftSingleTraitDisplay extends Vue {
    @Prop({required: true})
    trait!: TftTrait

    @Prop({required: true})
    tftSet!: string

    @Prop({default: 32})
    widthHeight!: number

    currentTrait: TftTraitData | null = null

    get parentStyle(): any {
        return {
            'width': `${this.widthHeight}px`,
            'height': `${this.widthHeight}px`,
            'position': 'relative'
        }
    }

    get traitStyle(): any {
        return {
            'width': `${this.widthHeight * 0.6}px`,
            'height': `${this.widthHeight * 0.6}px`,
            //@ts-ignore
            'background-image': `url('${this.traitImageUrl}')`,
            'background-size': 'contain',
            'background-position': 'center',
            'z-index': 1,
        }
    }

    get backgroundStyle(): any {
        return {
            'width': `${this.widthHeight}px`,
            'height': `${this.widthHeight}px`,
            //@ts-ignore
            'background-image': `url('${this.$root.generateAssetUri(this.traitBackgroundUrl)}')`,
            'background-size': 'contain',
            'background-position': 'center',
            'position': 'absolute',
        }
    }

    get traitImageUrl(): string {
        return staticClient.getTftTraitIcon(this.tftSet, this.trait.name)
    }

    get traitBackgroundUrl(): string {
        switch (this.trait.style) {
            case 1:
                return 'assets/tft/traits/bronze_clipped.png'
            case 2:
                return 'assets/tft/traits/silver_clipped.png'
            case 3:
                return 'assets/tft/traits/gold_clipped.png'
            case 4:
                return 'assets/tft/traits/chromatic_clipped.png'
        }
        return ''
    }

    @Watch('trait')
    @Watch('tftSet')
    refreshData() {
        axios.get(staticClient.getTftTraitData(this.tftSet, this.trait.name)).then((resp: ApiData<TftTraitData>) => {
            this.currentTrait = resp.data
        }).catch((err: any) => {
            console.log('Failed to get TFT trait data: ', err)
            this.currentTrait = {
                key: 'Unknown',
                name: 'Unknown'
            }
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>