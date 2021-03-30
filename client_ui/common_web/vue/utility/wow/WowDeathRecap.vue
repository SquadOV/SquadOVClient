<template>
    <loading-container :is-loading="!recap">
        <template v-slot:default="{ loading }">
            <div class="d-flex justify-center align-center">
                <v-icon>
                    mdi-skull
                </v-icon>

                <wow-character-display
                    class="mx-2 flex-grow-0 flex-shrink-1"
                    :character="character"
                    style="flex-basis: auto;"
                >
                </wow-character-display>

                <v-icon>
                    mdi-skull
                </v-icon>
            </div>

            <v-virtual-scroll
                :items="recap.hpEvents"
                bench="5"
                :height="800"
                item-height="64"
            >
                <template v-slot:default="{item}">
                    <v-list-item
                        dense
                    >
                        <v-list-item-content class="death-content">
                            <div class="d-flex align-center">
                                <div class="text-subtitle-2 font-weight-bold">
                                    Source: 
                                </div>

                                <template v-if="!!item.sourceGuid && characterGuidMapping.has(item.sourceGuid)">
                                    <wow-character-display
                                        :character="characterGuidMapping.get(item.sourceGuid)"
                                    >
                                    </wow-character-display>
                                </template>

                                <div class="text-subtitle-2" v-else>
                                    {{ item.sourceName }}
                                </div>
                            </div>
                        </v-list-item-content>
                        <div class="d-flex flex-grow-1 justify-center">
                            <v-list-item-content class="death-content">
                                <div class="text-subtitle-2 font-weight-bold">
                                    {{ secondsToTimeString(item.diffMs / 1000.0) }}
                                </div>
                            </v-list-item-content>

                            <v-list-item-content class="death-content mx-8">
                                <div class="d-flex align-center justify-center">
                                    <div :class="`text-subtitle-2 font-weight-bold ${(item.diffHp > 0) ? 'heal-text': 'damage-text'}`">
                                        {{ item.diffHp }}
                                    </div>

                                    <v-img
                                        class="mx-2"
                                        width="32"
                                        height="32"
                                        max-width="32"
                                        max-height="32"
                                        contain
                                        :src="$root.generateAssetUri(spellIdIcon(sanitizeSpellId(item.spellId)))"
                                    >
                                    </v-img>

                                    <div class="text-subtitle-2 font-weight-bold">
                                        {{ spellIdNames[sanitizeSpellId(item.spellId)] }}
                                    </div>
                                </div>
                            </v-list-item-content>

                            <v-list-item-action>
                                <v-btn
                                    outlined
                                    fab
                                    small
                                    color="white"
                                    @click="goToEvent(item)"
                                >
                                    <v-icon>mdi-play</v-icon>
                                </v-btn>
                            </v-list-item-action>
                        </div>
                    </v-list-item>
                </template>
            </v-virtual-scroll>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { WowDeath } from '@client/js/wow/events'
import { WowCharacter } from '@client/js/wow/character'
import { apiClient, ApiData } from '@client/js/api'
import { WowDeathRecap, WowDeathRecapEvent } from '@client/js/wow/deaths'
import { secondsToTimeString } from '@client/js/time'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import WowCharacterDisplay from '@client/vue/utility/wow/WowCharacterDisplay.vue'
import { staticClient } from '@client/js/staticData'
import { wowCache } from '@client/js/wow/staticCache'

const DEATH_RECAP_LENGTH_SECONDS: number = 5

@Component({
    components: {
        LoadingContainer,
        WowCharacterDisplay,
    }
})
export default class WowDeathRecapAnalysis extends Vue {
    secondsToTimeString = secondsToTimeString

    @Prop({required: true})
    matchUuid!: string

    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    character!: WowCharacter

    @Prop({required: true})
    matchCharacters!: WowCharacter[]

    @Prop({required: true})
    death!: WowDeath

    @Prop({required: true})    
    startTime!: Date

    recap: WowDeathRecap | null = null
    spellIdNames: {[id: number]: string} = {}

    get characterGuidMapping(): Map<string, WowCharacter> {
        let ret = new Map()
        for (let c of this.matchCharacters) {
            ret.set(c.guid, c)
        }
        return ret
    }

    goToEvent(e: WowDeathRecapEvent) {
        let tm = e.tm.getTime() + e.diffMs
        this.$emit('go-to-time', new Date(tm))
    }

    sanitizeSpellId(id: number | null): number {
        if (id === null) {
            return 6603 // auto attack
        } else {
            return id
        }
    }

    spellIdIcon(id: number): string {
        return staticClient.getWowSpellIconUrl(id)
    }

    @Watch('matchUuid')
    @Watch('userId')
    @Watch('death')
    refreshData() {
        apiClient.getWowDeathRecap(this.userId, this.matchUuid, this.death.eventId, DEATH_RECAP_LENGTH_SECONDS).then((resp: ApiData<WowDeathRecap>) => {
            this.recap = resp.data
            let uniqueSpellIds: Set<number> = new Set()
            for (let e of this.recap.hpEvents) {
                uniqueSpellIds.add(this.sanitizeSpellId(e.spellId))
            }

            wowCache.bulkGetSpellNames(Array.from(uniqueSpellIds)).then((resp: Map<number, string>) => {
                for (let [key, value] of resp) {
                    Vue.set(this.spellIdNames, key, value)
                }
            })

        }).catch((err: any) => {
            console.log('Failed to get death recap: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>

<style scoped>

.heal-text {
    color: green;
}

.damage-text {
    color: red;
}

.death-content {
    flex-grow: 0;
    flex-shrink: 1;
    flex-basis: auto;
}

</style>