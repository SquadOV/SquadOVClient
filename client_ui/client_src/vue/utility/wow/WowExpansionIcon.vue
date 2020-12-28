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
    WIDTH_MULTIPLIER = 1.92567

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
        }
        return ''
    }
}

</script>