<template>
    <div>
        {{ wins }} Wins {{ losses }} Losses <span>{{ winPercentage }}</span>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'


@Component
export default class WinLossStat extends Vue {
    @Prop({type: Number, required: true})
    wins!: number

    @Prop({type: Number, required: true})
    games!: number

    get losses() : number {
        return this.games - this.wins
    }

    get winPercentage() : string {
        if (this.games == 0) {
            return '100%'
        }
        return ((this.wins / this.games * 100.0).toFixed(2) + '%'
    }
}

</script>

<style scoped>

</style>