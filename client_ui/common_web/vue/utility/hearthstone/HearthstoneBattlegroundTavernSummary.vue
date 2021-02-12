<template>
    <v-tabs grow>
        <template v-for="level in 6">
            <v-tab
                :key="`tab-${level}`"
            >
                <v-img
                    :max-width="32"
                    contain
                    :src="$root.generateAssetUri(techLevelIcon(level))"
                >
                </v-img>
            </v-tab>

            <v-tab-item
                :key="`item-${level}`"
            >
                <hearthstone-battleground-tavern-tech-level-summary
                    :level="level"
                    :style="containerStyle"
                >
                </hearthstone-battleground-tavern-tech-level-summary>
            </v-tab-item>
        </template>
    </v-tabs>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import HearthstoneBattlegroundTavernTechLevelSummary from '@client/vue/utility/hearthstone/HearthstoneBattlegroundTavernTechLevelSummary.vue'

@Component({
    components: {
        HearthstoneBattlegroundTavernTechLevelSummary
    }
})
export default class HearthstoneBattlegroundTavernSummary extends Vue {
    @Prop({default: 100})
    height!: number

    techLevelIcon(level: number): string {
        return `assets/hearthstone/battlegrounds/tech_${level}.png`
    }

    get containerStyle() : any { 
        return {
            'height': `calc(${this.height}px - 48px)`,
            'overflow': 'auto'
        }
    }
}

</script>