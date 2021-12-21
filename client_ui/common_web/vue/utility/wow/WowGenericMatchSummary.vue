<template>
    <div>
        <router-link :to="to" :disabled="disabled" :event="disableLink ? '' : 'click'">
            <v-sheet
                :class="`d-flex align-center match-summary pa-2 ${fill ? 'full-parent-height' : ''}`"
                rounded
                :style="style"
            >
                <v-row no-gutters :class="`${fill ? 'full-parent-height' : ''}`" style="overflow: auto;">
                    <v-col :cols="mini ? 7 : 5" align-self="center">
                        <slot v-bind="{ instanceName }"></slot>

                        <div class="text-subtitle-1">
                            {{ timestamp }}
                        </div>

                        <div class="text-subtitle-2" v-if="elapsedTime.length > 0">
                            Finished in: {{ elapsedTime }}
                        </div>
                    </v-col>

                    <v-col :cols="5" v-if="!!relevantCharacters" align-self="center">
                        <div class="d-flex align-center flex-wrap">
                            <template v-if="showFullCharacters || !povCharacter">
                                <wow-character-icon
                                    v-for="(char, idx) in friendlyCharacters"
                                    :char="char"
                                    :friendly-team="useTeams ? friendlyTeam : char.team"
                                    armory-link
                                    :player-section="linkToPlayerSection"
                                    :key="`friendly-icon-${idx}`"
                                    :width-height="mini ? 24 : 32"
                                    :patch="match.build"
                                    :is-self="!!povCharacter ? char.guid === povCharacter.guid : false"
                                    @go-to-character="$emit('go-to-character', arguments[0])"
                                >
                                </wow-character-icon>

                                <template v-if="useTeams">
                                    <div
                                        v-if="enemyCharacters.length > 0"
                                        class="mx-1 text-overline"
                                    >
                                        VS
                                    </div>

                                    <wow-character-icon
                                        v-for="(char, idx) in enemyCharacters"
                                        :char="char"
                                        :friendly-team="friendlyTeam"
                                        armory-link
                                        :player-section="linkToPlayerSection"
                                        :key="`enemy-icon-${idx}`"
                                        :width-height="mini ? 24 : 32"
                                        :patch="match.build"
                                        :is-self="!!povCharacter ? char.guid === povCharacter.guid : false"
                                        @go-to-character="$emit('go-to-character', arguments[0])"
                                    >
                                    </wow-character-icon>
                                </template>
                            </template>

                            <template v-else>
                                <div class="mr-1">
                                    POV: 
                                </div>

                                <wow-character-icon
                                    is-self
                                    :char="povCharacter"
                                    :friendly-team="friendlyTeam"
                                    armory-link
                                    :player-section="linkToPlayerSection"
                                    :key="`same-squad-icon`"
                                    :width-height="mini ? 24 : 32"
                                    :patch="match.build"
                                    @go-to-character="$emit('go-to-character', arguments[0])"
                                >
                                </wow-character-icon>
                            </template>
                        </div>

                        <slot name="extra"></slot>
                    </v-col>

                    <v-col :cols="2" v-if="!mini" align-self="center">
                        <div class="d-flex align-center">
                            <wow-expansion-icon v-if="!!instanceData"
                                class="mr-4"
                                :expansion-id="instanceData.expansion"
                                :height="48"
                            >
                            </wow-expansion-icon>

                            <div class="text-overline">
                                {{ match.build }}
                            </div>
                        </div>
                    </v-col>
                </v-row>

                <div class="vod-div" v-if="hasVod">
                    <v-icon color="black">
                        mdi-video
                    </v-icon>
                </div>
            </v-sheet>
        </router-link>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { WowCommonMatch } from '@client/js/wow/matches'
import { VodAssociation } from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import { staticClient } from '@client/js/staticData'
import {
    WowCharacter,
    WoWCharacterUserAssociation
} from '@client/js/wow/character'
import { WowInstanceData } from '@client/js/wow/instance'
import { standardFormatTime } from '@client/js/time'
import WowExpansionIcon from '@client/vue/utility/wow/WowExpansionIcon.vue'
import WowCharacterIcon from '@client/vue/utility/wow/WowCharacterIcon.vue'
import axios from 'axios'
import { compareString } from '@client/js/cmp'
import * as pi from '@client/js/pages'

@Component({
    components: {
        WowCharacterIcon,
        WowExpansionIcon
    }
})
export default class WowGenericMatchSummary extends Vue {
    @Prop({required: true})
    match!: WowCommonMatch

    @Prop({type: Number, required: true})
    userId!: number

    @Prop({type: String, required: true})
    elapsedTime!: string

    @Prop({type: Boolean, default: false})
    mini!: boolean

    @Prop({type: Boolean, default: false})
    fill!: boolean

    @Prop({type: Boolean, default: false})
    useTeams!: boolean

    @Prop({default: 0})
    friendlyTeam!: number

    @Prop({type: Boolean, default: false})
    disableLink!: boolean

    @Prop({type: Boolean, default: false})
    linkToPlayerSection!: boolean

    @Prop({type: Boolean, default: false})
    showFullCharacters!: boolean

    @Prop()
    accessToken!: string | undefined

    vod: VodAssociation | null = null
    relevantCharacters: WowCharacter[] | null = null
    instanceData: WowInstanceData | null = null
    characterAssociations: WoWCharacterUserAssociation[] = []

    get friendlyCharacters(): WowCharacter[] {
        if (!this.relevantCharacters) {
            return []
        }

        if (!this.useTeams) {
            return this.relevantCharacters
        }

        return this.relevantCharacters.filter((c: WowCharacter) => {
            return c.team == this.friendlyTeam
        })
    }

    get enemyCharacters(): WowCharacter[] {
        if (!this.relevantCharacters) {
            return []
        }

        if (!this.useTeams) {
            return []
        }

        return this.relevantCharacters.filter((c: WowCharacter) => {
            return c.team != this.friendlyTeam
        })
    }

    get povCharacter(): WowCharacter | null {
        if (!this.relevantCharacters) {
            return null
        }

        let okChars = new Set(this.characterAssociations.filter((ele: WoWCharacterUserAssociation) => ele.userId === this.userId).map((ele: WoWCharacterUserAssociation) => ele.guid))
        let filtered = this.relevantCharacters.filter((ele: WowCharacter) => okChars.has(ele.guid))
        if (filtered.length > 0) {
            return filtered[0]
        } else {
            return null
        }
    }

    get hasVod() : boolean {
        return !!this.vod
    }

    get timestamp(): string {
        return standardFormatTime(this.match.tm)
    }

    get instanceName(): string {
        return !!this.instanceData ? this.instanceData.name : 'Loading...'
    }

    get style() : any {
        let wlColor = this.match.success ? 'color-top-place' : 'color-bottom-place'
        let style: any = {
            'border-left': `5px solid var(--${wlColor})`,
            'background-position': 'right',
            'background-size': this.mini ? 'auto 150%' : '50% auto',
        }

        style['background-image'] = `linear-gradient(to right, #1E1E1E 0 10%, transparent), url(${staticClient.getWowInstanceBackgroundUrl(this.match.build, this.match.instanceId)})`
        return style
    }

    get disabled(): boolean {
        return this.$route.name === pi.WowMatchPageId || this.disableLink
    }

    get to() : any {
        return {
            name: pi.WowMatchPageId,
            params: {
                matchUuid: this.match.matchUuid
            },
            query: {
                userId: this.userId,
                at: this.accessToken,
            }
        }
    }

    @Watch('match')
    @Watch('userId')
    refreshVod() {
        apiClient.accessToken(this.accessToken).findVodFromMatchUserId(this.match.matchUuid, this.userId).then((resp : ApiData<VodAssociation>) => {
            this.vod = resp.data
        }).catch((err : any) => {
            this.vod = null
        })
    }

    @Watch('match')
    @Watch('userId')
    refreshCharacterAssociations() {
        this.characterAssociations = []
        apiClient.accessToken().listWoWMatchCharacterAssociations(this.userId, this.match.matchUuid).then((resp: ApiData<WoWCharacterUserAssociation[]>) => {
            this.characterAssociations = resp.data
        }).catch((err: any) => {
            console.error('Failed to obtain WoW character associations: ', err)
        })
    }

    @Watch('match')
    @Watch('userId')
    refreshCharacters() {
        this.relevantCharacters = null
        apiClient.accessToken(this.accessToken).listWoWCharactersForMatch(this.match.matchUuid, this.userId).then((resp: ApiData<WowCharacter[]>) => {
            this.relevantCharacters = resp.data
            this.relevantCharacters!.sort((a: WowCharacter, b: WowCharacter) => {
                return (a.team - b.team) || compareString(a.name, b.name)
            })
        }).catch((err: any) => {
            console.error('Failed to get characters for WoW match: ', err)
        })
    }

    @Watch('match')
    refreshInstanceData() {
        let url = staticClient.getWowInstanceDataUrl(this.match.build, this.match.instanceId)
        axios.get(url).then((resp: ApiData<WowInstanceData>) => {
            this.instanceData = resp.data
        }).catch((err: any) => {
            console.error('Failed to obtain instance data: ', err)
        })
    }

    mounted() {
        this.refreshVod()
        this.refreshCharacters()
        this.refreshCharacterAssociations()
        this.refreshInstanceData()
    }
}

</script>

<style scoped>

.match-summary {
    width: 100%;
    position: relative;
}

</style>