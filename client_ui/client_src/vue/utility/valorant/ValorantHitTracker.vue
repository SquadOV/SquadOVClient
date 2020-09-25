<template>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'

@Component
export default class ValorantHitTracker extends Vue {
    @Prop({type : Number, default: 0})
    headshots! : number

    @Prop({type : Number, default: 0})
    bodyshots! : number

    @Prop({type : Number, default: 0})
    legshots! : number

    get totalShots() : number {
        return this.headshots + this.bodyshots + this.legshots
    }

    get hsp() : number {
        if (this.totalShots == 0) {
            return 0;
        }
        return this.headshots / this.totalShots
    }

    get bsp() : number {
        if (this.totalShots == 0) {
            return 0;
        }
        return this.bodyshots / this.totalShots
    }

    get lsp() : number {
        if (this.totalShots == 0) {
            return 0;
        }
        return this.legshots / this.totalShots
    }
}

</script>

<style scoped>

</style>