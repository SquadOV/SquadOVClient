<template>
    <div @click="goToArmoryLink">
        <v-tooltip
            offset-x right
        >
            <template v-slot:activator="{on, attrs}">
                <div
                    v-bind="attrs"
                    v-on="on"
                    :class="`ma-1 ${(char.team === friendlyTeam) ? 'friendly-char' : 'enemy-char'}`"
                >
                    <wow-class-spec-icon
                        v-if="char.specId > 0"
                        :spec-id="char.specId"
                        :width-height="widthHeight"
                        :patch="patch"
                    >
                    </wow-class-spec-icon>

                    <wow-class-icon
                        v-else-if="!!char.classId"
                        :class-id="char.classId"
                        :width-height="widthHeight"
                        :patch="patch"
                    >
                    </wow-class-icon>
                </div>
            </template>

            <wow-character-display
                :character="char"
                :patch="patch"
            >
            </wow-character-display>    
        </v-tooltip>
    </div>

</template>


<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { WowCharacter } from '@client/js/wow/character'
import { apiClient, ApiData } from '@client/js/api'
import { openUrlInBrowser } from '@client/js/external'
import WowClassSpecIcon from '@client/vue/utility/wow/WowClassSpecIcon.vue'
import WowClassIcon from '@client/vue/utility/wow/WowClassIcon.vue'
import WowCharacterDisplay from '@client/vue/utility/wow/WowCharacterDisplay.vue'
import { doesWowPatchSupportArmory } from '@client/js/wow/constants'

@Component({
    components: {
        WowClassSpecIcon,
        WowClassIcon,
        WowCharacterDisplay
    }
})
export default class WowCharacterIcon extends Vue {
    @Prop({required: true})
    char!: WowCharacter

    @Prop({type: Boolean, default: false})
    armoryLink!: boolean

    @Prop({type: Boolean, default: false})
    playerSection!: boolean

    @Prop({default: 0})
    friendlyTeam!: number

    @Prop({type: Number, default: 32})
    widthHeight!: number

    @Prop({required: true})
    patch!: string

    goToArmoryLink(e: MouseEvent) {
        if (!this.armoryLink || !doesWowPatchSupportArmory(this.patch)) {
            return
        }

        e.preventDefault()
        e.stopPropagation()

        if (!this.playerSection) {
            // Use the api client to get the proper URL to redirect to as the server will know how to map
            // the character GUID to an armory link.
            apiClient.accessToken().getWoWCharacterArmoryLink(this.char.name, this.char.guid).then((resp: ApiData<string>) => {
                openUrlInBrowser(resp.data)
            }).catch((err: any) => {
                console.error('Failed to get WoW character armory link: ', err)
            })
        } else {
            this.$emit('go-to-character', this.char.guid)
        }
    }

    divStyle(): any {
        if (this.armoryLink) {
            return {
                cursor: 'pointer'
            }
        } else {
            return {}
        }
    }
}

</script>

<style scoped>

.friendly-char {
    border: 2px solid var(--color-friendly) !important;
}

.enemy-char {
    border: 2px solid var(--color-enemy) !important;
}

</style>