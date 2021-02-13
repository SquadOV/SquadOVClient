<template>
    <v-img
        :src="$root.generateAssetUri(iconUrl)"
        :max-width="widthHeight"
        :max-height="widthHeight"
        contain
    >
    </v-img>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { ApiData } from '@client/js/api'
import { staticClient } from '@client/js/staticData'
import { WowClassSpecData, WowClassData } from '@client/js/wow/character'
import axios from 'axios'

@Component
export default class WowClassIcon extends Vue {
    iconUrl: string = 'assets/wow/Unknown.png'
    spec: WowClassSpecData | null = null
    class: WowClassData | null = null

    @Prop({type: Number})
    specId!: number

    @Prop({type: Number, default: 32})
    widthHeight!: number

    @Watch('specId')
    refreshData() {
        let specUrl = staticClient.getWowSpecsDataUrl(this.specId)
        axios.get(specUrl).then((resp: ApiData<WowClassSpecData>) => {
            this.spec = resp.data

            axios.get(staticClient.getWowClassDataUrl(this.spec!.class)).then((resp: ApiData<WowClassData>) => {
                this.class = resp.data
            }).catch((err: any) => {
                console.log('Failed to get class: ', err)
            })

            this.iconUrl = staticClient.getWowClassIconUrl(this.spec!.class)
        }).catch((err: any) => {
            console.log('Failed to get spec: ', err)
            this.iconUrl = 'assets/wow/Unknown.png'
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>