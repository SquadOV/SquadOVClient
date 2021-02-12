<template>
    <div>
        <router-link :to="to">
            <v-sheet
                :class="`d-flex align-center match-summary pa-2 ${fill ? 'full-parent-height' : ''}`"
                rounded
                :style="style"
            >
                <v-row no-gutters :class="`${fill ? 'full-parent-height' : ''}`">
                    <v-col :cols="mini ? 4 : 2" align-self="center">
                        <slot v-bind="{ instanceName }"></slot>
                    </v-col>

                    <v-col :cols="mini ? 5 : 3" align-self="center">
                        <div class="text-h6">
                            {{ timestamp }}
                        </div>

                        <div class="text-subtitle-2">
                            Finished in: {{ elapsedTime }}
                        </div>
                    </v-col>

                    <v-col cols="5" v-if="!!relevantCharacters && !mini" align-self="center">
                        <div class="d-flex align-center flex-wrap">
                            <template v-for="(char, idx) in relevantCharacters">
                                <v-tooltip
                                    offset-x right
                                    :key="`tooltip-${idx}`"
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

                                <span
                                    v-if="useTeams && idx < relevantCharacters.length - 1 && relevantCharacters[idx].team != relevantCharacters[idx+1].team"
                                    :key="`vs-${idx}`"
                                    class="mx-1 text-overline"
                                >
                                    VS
                                </span>
                            </template>
                        </div>
                    </v-col>

                    <v-col :cols="mini ? 3 : 2" align-self="center">
                        <div class="d-flex align-center">
                            <wow-expansion-icon v-if="!!instanceData"
                                class="mr-4"
                                :expansion-id="instanceData.expansion"
                                :height="48"
                            >
                            </wow-expansion-icon>

                            <div class="text-overline" v-if="!mini">
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
import {
    getSuccessColor,
    getFailureColor
} from '@client/js/wow/colors'
import { VodAssociation } from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import { staticClient } from '@client/js/staticData'
import { WowCharacter } from '@client/js/wow/character'
import { WowInstanceData } from '@client/js/wow/instance'
import { standardFormatTime } from '@client/js/time'
import WowClassSpecIcon from '@client/vue/utility/wow/WowClassSpecIcon.vue'
import WowCharacterDisplay from '@client/vue/utility/wow/WowCharacterDisplay.vue'
import WowExpansionIcon from '@client/vue/utility/wow/WowExpansionIcon.vue'
import axios from 'axios'
import * as pi from '@client/js/pages'

@Component({
    components: {
        WowClassSpecIcon,
        WowCharacterDisplay,
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

    vod: VodAssociation | null = null
    relevantCharacters: WowCharacter[] | null = null
    instanceData: WowInstanceData | null = null

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
        let wlColor = this.match.success ? getSuccessColor() : getFailureColor()
        let style: any = {
            'border-left': `5px solid rgb(${wlColor.r}, ${wlColor.g}, ${wlColor.b})`,
            'background-position': 'right',
            'background-size': this.mini ? 'auto 150%' : '50% auto',
        }

        style['background-image'] = `linear-gradient(to right, #1E1E1E 0 10%, transparent), url(${staticClient.getWowInstanceBackgroundUrl(this.match.instanceId)})`
        return style
    }

    get to() : any {
        if (this.$route.name === pi.WowMatchPageId) {
            return {}
        }

        return {
            name: pi.WowMatchPageId,
            params: {
                matchUuid: this.match.matchUuid
            },
            query: {
                userId: this.userId,
            }
        }
    }

    @Watch('match')
    @Watch('userId')
    refreshVod() {
        apiClient.findVodFromMatchUserId(this.match.matchUuid, this.userId).then((resp : ApiData<VodAssociation>) => {
            this.vod = resp.data
        }).catch((err : any) => {
            this.vod = null
        })
    }

    @Watch('match')
    @Watch('userId')
    refreshCharacters() {
        this.relevantCharacters = null
        apiClient.listWoWCharactersForMatch(this.match.matchUuid, this.userId).then((resp: ApiData<WowCharacter[]>) => {
            this.relevantCharacters = resp.data
            this.relevantCharacters!.sort((a: WowCharacter, b: WowCharacter) => {
                return a.team - b.team
            })
        }).catch((err: any) => {
            console.log('Failed to get characters for WoW match: ', err)
        })
    }

    @Watch('match')
    refreshInstanceData() {
        let url = staticClient.getWowInstanceDataUrl(this.match.instanceId)
        axios.get(url).then((resp: ApiData<WowInstanceData>) => {
            this.instanceData = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain instance data: ', err)
        })
    }

    mounted() {
        this.refreshVod()
        this.refreshCharacters()
        this.refreshInstanceData()
    }
}

</script>

<style scoped>

.match-summary {
    width: 100%;
    position: relative;
}

.friendly-char {
    border: 2px solid rgb(76, 175, 80) !important;
}

.enemy-char {
    border: 2px solid rgb(255, 82, 82) !important;
}

</style>