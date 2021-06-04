<template>
    <div>
        <div class="d-flex align-center px-8">
            <div class="text-subtitle-2 font-weight-bold">
                {{ character.rating }}
            </div>

            <wow-character-display
                :character="character"
                :patch="patch"
            >
            </wow-character-display>

            <v-spacer></v-spacer>
                
            <v-img
                @click="goToArmory"
                :width="81"
                :max-width="81"
                :height="48"
                :max-height="48"
                contain
                :src="$root.generateAssetUri('assets/wow/armory.png')"
                style="cursor: pointer;"
            >
            </v-img>
        </div>

        <v-divider class="mb-2"></v-divider>

        <loading-container :is-loading="!fullChar">
            <template v-slot:default="{ loading }">
                <v-container fluid v-if="!loading">
                    <v-row>
                        <v-col cols="6" class="px-4">
                            <div class="d-flex align-center">
                                <div class="text-h6 font-weight-bold mr-2">Class: </div>
                                <div class="text-h6 mr-2" :style="specTextStyling">{{ className }}</div>
                                <v-img
                                    :width="32"
                                    :max-width="32"
                                    :height="32"
                                    :max-height="32"
                                    contain
                                    :src="classIcon"
                                    v-if="!!classIcon"
                                >
                                </v-img>
                            </div>

                            <div class="d-flex align-center mt-2">
                                <div class="text-h6 font-weight-bold mr-2">Specialization: </div>
                                <div class="text-h6 mr-2" :style="specTextStyling">{{ specName }}</div>
                                <v-img
                                    :width="32"
                                    :max-width="32"
                                    :height="32"
                                    :max-height="32"
                                    contain
                                    :src="specIcon"
                                    v-if="!!specIcon"
                                >
                                </v-img>
                            </div>

                            <div class="d-flex align-center mt-2">
                                <div class="text-h6 font-weight-bold mr-2">Item Level: </div>
                                <div class="text-h6">{{ character.ilvl }}</div>
                            </div>

                            <div class="d-flex align-center mt-2">
                                <div class="text-h6 font-weight-bold mr-2">Rating: </div>
                                <div class="text-h6">{{ character.rating }}</div>
                            </div>

                            <div class="mt-2 d-flex align-center">
                                <div class="text-h6 font-weight-bold mr-2">Class Talents: </div>
                                <wow-spell-icon
                                    class="mr-1"
                                    v-for="(id, idx) in fullChar.talents"
                                    :spell-id="id"
                                    :key="`class-talent-${idx}`"
                                    :patch="patch"
                                >
                                </wow-spell-icon>
                            </div>

                            <div class="mt-2 d-flex align-center">
                                <div class="text-h6 font-weight-bold mr-2">PVP Talents: </div>
                                <wow-spell-icon
                                    class="mr-1"
                                    v-for="(id, idx) in fullChar.pvpTalents"
                                    :spell-id="id"
                                    :key="`pvp-talent-${idx}`"
                                    :patch="patch"
                                >
                                </wow-spell-icon>
                            </div>

                            <div class="mt-2" v-if="!!fullChar.covenant">
                                <wow-covenant-display
                                    :covenant="fullChar.covenant"
                                    :patch="patch"
                                >
                                </wow-covenant-display>
                            </div>
                        </v-col>

                        <v-col cols="6" class="px-4">
                            <div class="text-h6 font-weight-bold">
                                Items
                            </div>
                            <v-divider></v-divider>

                            <wow-character-items-display
                                :items="fullChar.items"
                                :patch="patch"
                            >
                            </wow-character-items-display>
                        </v-col>
                    </v-row>
                </v-container>
            </template>
        </loading-container>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { WowCharacter, WowFullCharacter } from '@client/js/wow/character'
import { apiClient, ApiData } from '@client/js/api'
import { openUrlInBrowser } from '@client/js/external'
import { wowCache, WowClassSpecStatic, WowClassStatic } from '@client/js/wow/staticCache'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import WowCharacterDisplay from '@client/vue/utility/wow/WowCharacterDisplay.vue'
import WowCharacterItemsDisplay from '@client/vue/utility/wow/WowCharacterItemsDisplay.vue'
import WowSpellIcon from '@client/vue/utility/wow/WowSpellIcon.vue'
import WowCovenantDisplay from '@client/vue/utility/wow/WowCovenantDisplay.vue'
import { staticClient } from '@client/js/staticData'
import { specIdToColor } from '@client/js/wow/colors'

@Component({
    components: {
        LoadingContainer,
        WowCharacterDisplay,
        WowCharacterItemsDisplay,
        WowSpellIcon,
        WowCovenantDisplay
    }
})
export default class WowCharacterFullDisplay extends Vue {
    @Prop({required: true})
    matchUuid!: string

    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    character!: WowCharacter

    @Prop({required: true})
    patch!: string

    fullChar: WowFullCharacter | null = null
    armoryProgress: boolean = false

    specName: string | null = null
    specIcon: string | null = null
    className: string | null = null
    classIcon: string | null = null

    goToArmory() {
        this.armoryProgress = true
        apiClient.getWoWCharacterArmoryLink(this.character.name, this.character.guid).then((resp: ApiData<string>) => {
            openUrlInBrowser(resp.data)
        }).catch((err: any) => {
            console.log('Failed to get WoW character armory link: ', err)
        }).finally(() => {
            this.armoryProgress = false
        })
    }

    @Watch('matchUuid')
    @Watch('userId')
    @Watch('character')
    refreshData() {
        apiClient.getWowFullCharacter(this.userId, this.matchUuid, this.character.guid).then((resp: ApiData<WowFullCharacter>) => {
            this.fullChar = resp.data
        }).catch((err: any) => {
            console.log('Failed to get full WoW character info: ', err)
        })

        this.specIcon = staticClient.getWowSpecsIconUrl(this.patch, this.character.specId)
        wowCache.getCache(this.patch).getClassSpec(this.character.specId).then((data: WowClassSpecStatic) => {
            this.specName = data.name
            this.classIcon = staticClient.getWowClassIconUrl(this.patch, data.class)
            wowCache.getCache(this.patch).getClass(data.class).then((classData: WowClassStatic) => {
                this.className = classData.name
            })
        }).catch((err: any) => {
            console.log('Failed to get WoW spec: ', err)
        })
    }

    get specTextStyling(): any {
        return {
            color: `var(--${specIdToColor(this.character.specId)})`
        }
    }

    mounted() {
        this.refreshData()
    }
}

</script>