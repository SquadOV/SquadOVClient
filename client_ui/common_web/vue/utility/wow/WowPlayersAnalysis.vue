<template>
    <v-tabs v-model="tab" vertical>
        <template v-for="(char, idx) in matchCharacters">
            <v-tab :key="`tab-${idx}`" :style="charStyle(char)">
                <wow-character-display
                    :character="char"
                    :patch="patch"
                >
                </wow-character-display>
            </v-tab>

            <v-tab-item :key="`item-${idx}`">
                <wow-character-full-display
                    :character="char"
                    :match-uuid="matchUuid"
                    :user-id="userId"
                    :patch="patch"
                >
                </wow-character-full-display>
            </v-tab-item>
        </template>
    </v-tabs>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { WowCharacter } from '@client/js/wow/character'
import WowCharacterFullDisplay from '@client/vue/utility/wow/WowCharacterFullDisplay.vue'
import WowCharacterDisplay from '@client/vue/utility/wow/WowCharacterDisplay.vue'

@Component({
    components: {
        WowCharacterFullDisplay,
        WowCharacterDisplay,
    }
})
export default class WowPlayersAnalysis extends Vue {
    @Prop({required: true})
    matchUuid!: string

    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    matchCharacters!: WowCharacter[]

    @Prop({default: 0})
    friendlyTeam!: number

    @Prop({required: true})
    patch!: string

    @Prop()
    initialCharacterGuid!: string | undefined

    tab: number = 0

    charStyle(char: WowCharacter): any {
        return {
            'border-right': `2px solid var(--${char.team === this.friendlyTeam ? 'color-top-place' : 'color-bottom-place'})`
        }
    }

    @Watch('initialCharacterGuid')
    refreshViewCharacter() {
        let charIdx = this.matchCharacters.findIndex((ele: WowCharacter) => ele.guid === this.initialCharacterGuid)
        if (charIdx == -1) {
            return
        }
        this.tab = charIdx
    }

    mounted() {
        this.refreshViewCharacter()
    }
}

</script>