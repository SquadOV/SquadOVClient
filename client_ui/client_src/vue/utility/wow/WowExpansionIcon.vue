<template>
    <v-img
        :src="src"
        class="flex-grow-0"
        :height="height"
        :width="height * WIDTH_MULTIPLIER"
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
export default class WowExpansionIcon extends Vue {
    WIDTH_MULTIPLIER = 2

    @Prop()
    expansionId!: number | string

    @Prop({type: Number, default: 64})
    height!: number

    get src(): string {
        let expansion = Number(this.expansionId)
        switch (expansion) {
            case 8:
                return 'assets/wow/expansions/Shadowlands.png'
            case 7:
                return 'assets/wow/expansions/BfA.png'
            case 6:
                return 'assets/wow/expansions/Legion.png'
            case 3:
                return 'assets/wow/expansions/Cataclysm.png'
            case 1:
                return 'assets/wow/expansions/BurningCrusade.png'
            case 0:
                return 'assets/wow/expansions/Classic.png'
        }
        return ''
    }
}

</script>