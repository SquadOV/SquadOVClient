<template>
    <v-list-item
        two-line
        dense
    >
        <v-list-item-content class="content mr-4">
            <wow-class-spec-icon
                v-if="character.specId > 0"
                :spec-id="character.specId"
                :patch="patch"
            >
            </wow-class-spec-icon>

            <wow-class-icon
                v-else-if="!!character.classId"
                :class-id="character.classId"
                :patch="patch"
            >
            </wow-class-icon>
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
import WowClassIcon from '@client/vue/utility/wow/WowClassIcon.vue'

@Component({
    components: {
        WowClassSpecIcon,
        WowClassIcon,
    }
})
export default class WowCharacterDisplay extends Vue {
    @Prop({required: true})
    character!: WowCharacter

    @Prop({required: true})
    patch!: string

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