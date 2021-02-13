<template>
    <v-img
        :src="src"
        :width="width"
        :height="height"
        :max-width="width"
        :max-height="height"
    >
    </v-img>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { ddragonContainer } from '@client/js/lolDdragon'

@Component
export default class LolChampionIcon extends Vue {
    @Prop({required: true})
    championId!: number

    @Prop({required: true})
    gameVersion!: string

    @Prop({default: 128})
    width!: number

    @Prop({default: 128})
    height!: number

    src: string = ''

    @Watch('championId')
    refreshSrc() {
        this.src = ''
        ddragonContainer.getClientForVersion(this.gameVersion).getLolChampionIconUrl(this.championId).then((resp: string) => {
            this.src = resp
        }).catch((err: any) => {
            console.log('Failed to get LoL champion icon: ', err)
        })
    }

    mounted() {
        this.refreshSrc()
    }
}

</script>