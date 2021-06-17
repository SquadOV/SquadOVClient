<template>
    <v-select
        :value="value"
        @input="$emit('input', arguments[0])"
        label="WoW Character"
        :items="items"
        :multiple="multiple"
        solo
        hide-details
    >
        <template v-slot:selection="{item}">
            <wow-character-display
                :character="item"
                :patch="patch"
                v-if="!multiple"
            >
            </wow-character-display>

            <v-chip class="mr-1" v-else>
                {{ item.name }}
            </v-chip>
        </template>

        <template v-slot:item="{item}">
            <div class="d-flex full-width align-center">
                <v-checkbox
                    class="selection-checkbox"
                    dense
                    hide-details
                    :input-value="value.includes(item)"
                    readonly
                    v-if="multiple && Array.isArray(value)"
                >
                </v-checkbox>

                <wow-character-display
                    :character="item"
                    :patch="patch"
                >
                </wow-character-display>
            </div>
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
    value!: WowCharacter | WowCharacter[] | null | undefined

    @Prop({required: true})
    patch!: string

    @Prop({type: Boolean, default: false})
    multiple!: boolean

    get items(): any[] {
        return this.characters
    }
}

</script>