<template>
    <div class="d-flex align-center">
        <lol-item-icon
            v-for="(id, idx) in itemIds"
            :key="idx"
            :width-height="itemWidthHeight"
            :item-id="id"
            :game-version="gameVersion"
        >
        </lol-item-icon>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { LolParticipantStats } from '@client/js/lol/participant'
import LolItemIcon from '@client/vue/utility/lol/LolItemIcon.vue'

@Component({
    components: {
        LolItemIcon
    }
})
export default class LolPlayerItems extends Vue {
    @Prop({required: true})
    stats!: LolParticipantStats

    @Prop({default: 32})
    itemWidthHeight!: number

    @Prop({required: true})
    gameVersion!: string

    get itemIds(): number[] {
        return [
            this.stats.item0,
            this.stats.item1,
            this.stats.item2,
            this.stats.item3,
            this.stats.item4,
            this.stats.item5,
            this.stats.item6,
        ]
    }
}

</script>