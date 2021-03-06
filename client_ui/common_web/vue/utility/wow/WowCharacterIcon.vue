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
                        :spec-id="char.specId"
                    >
                    </wow-class-spec-icon>    
                </div>
            </template>

            <wow-character-display
                :character="char"
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
import WowCharacterDisplay from '@client/vue/utility/wow/WowCharacterDisplay.vue'

@Component({
    components: {
        WowClassSpecIcon,
        WowCharacterDisplay
    }
})
export default class WowCharacterIcon extends Vue {
    @Prop({required: true})
    char!: WowCharacter

    @Prop({type: Boolean, default: false})
    armoryLink!: boolean

    @Prop({default: 0})
    friendlyTeam!: number

    goToArmoryLink(e: MouseEvent) {
        if (!this.armoryLink) {
            return
        }

        e.preventDefault()
        e.stopPropagation()

        // Use the api client to get the proper URL to redirect to as the server will know how to map
        // the character GUID to an armory link.
        apiClient.getWoWCharacterArmoryLink(this.char.name, this.char.guid).then((resp: ApiData<string>) => {
            openUrlInBrowser(resp.data)
        }).catch((err: any) => {
            console.log('Failed to get WoW character armory link: ', err)
        })
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
    border: 2px solid rgb(76, 175, 80) !important;
}

.enemy-char {
    border: 2px solid rgb(255, 82, 82) !important;
}

</style>