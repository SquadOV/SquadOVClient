<template>
    <v-list-item
        two-line
        dense
    >
        <v-list-item-content class="content mr-4">
            <wow-class-spec-icon
                :spec-id="character.specId"
            >
            </wow-class-spec-icon>
        </v-list-item-content>

        <v-list-item-content class="content">
            <v-list-item-title>
                {{ name }}
            </v-list-item-title>

            <v-list-item-subtitle>
                {{ server }} ({{character.ilvl}})
            </v-list-item-subtitle>
        </v-list-item-content>
    </v-list-item>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { WowCharacter } from '@client/js/wow/character'
import WowClassSpecIcon from '@client/vue/utility/wow/WowClassSpecIcon.vue'

@Component({
    components: {
        WowClassSpecIcon
    }
})
export default class WowCharacterDisplay extends Vue {
    @Prop({required: true})
    character!: WowCharacter

    get name(): string {
        return this.character.name.split('-')[0]
    }

    get server(): string {
        return this.character.name.split('-')[1]
    }

}

</script>

<style scoped>

.content {
    flex-grow: 0 !important;
    flex-basis: auto !important;
}

</style>