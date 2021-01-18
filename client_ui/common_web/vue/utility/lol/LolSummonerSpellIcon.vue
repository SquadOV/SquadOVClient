<template>
    <v-img
        :src="src"
        :width="widthHeight"
        :height="widthHeight"
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
import { ddragonContainer } from '@client/js/lolDdragon'

@Component
export default class LolLaneIcon extends Vue {
    @Prop({required: true})
    spellId!: number

    @Prop({required: true})
    gameVersion!: string

    @Prop({default: 16})
    widthHeight!: number

    src: string = ''

    @Watch('spellId')
    refreshSrc() {
        this.src = ''
        ddragonContainer.getClientForVersion(this.gameVersion).getLolSummonerSpellIconUrl(this.spellId).then((resp: string) => {
            this.src = resp
        }).catch((err: any) => {
            console.log('Failed to get LoL summoner spell icon: ', err)
        })
    }

    mounted() {
        this.refreshSrc()
    }
}

</script>