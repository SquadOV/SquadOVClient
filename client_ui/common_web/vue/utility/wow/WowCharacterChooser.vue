<template>
    <v-select
        :value="value"
        @input="$emit('input', arguments[0])"
        label="WoW Character"
        :items="items"
        solo
        hide-details
    >
        <template v-slot:selection="{item}">
            <wow-character-display
                :character="item"
            >
            </wow-character-display>
        </template>

        <template v-slot:item="{item}">
            <wow-character-display
                :character="item"
            >
            </wow-character-display>
        </template>
    </v-select>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { WowCharacter } from '@client/js/wow/character'
import WowCharacterDisplay from '@client/vue/utility/wow/WowCharacterDisplay.vue'

@Component({
    components: {
        WowCharacterDisplay
    }
})
export default class WowCharacterChooser extends Vue {
    @Prop({type: Array, required: true})
    characters!: WowCharacter[]

    @Prop({required: true})
    value!: WowCharacter | null | undefined

    get items(): any[] {
        return this.characters
    }
}

</script>