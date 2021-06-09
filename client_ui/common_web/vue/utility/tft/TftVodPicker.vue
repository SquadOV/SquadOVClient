<template>
    <generic-vod-picker
        :value="vod"
        @input="selectVod"
        :options.sync="allPovs"
        :match-uuid="matchUuid"
        :timestamp="timestamp"
        :game="game"
        :disable-favorite="disableFavorite"
        :enable-draw="enableDraw"
        @update:enableDraw="$emit('update:enableDraw', arguments[0])"
    >
        <template v-slot:vod="{ivod, selected}">
            <div
                :class="(!!selected && ivod.videoUuid === selected.videoUuid) ? 'selected-pov' : 'pov'"
            >
                <v-tooltip bottom>
                    <template v-slot:activator="{on, attrs}">
                        <div
                            v-on="on"
                            v-bind="attrs"
                        >
                            <tft-little-legend-icon
                                :content-id="uuidToParticipant(ivod.userUuid).companion.contentId"
                                :width="48"
                                :height="48"
                            >
                            </tft-little-legend-icon>
                        </div>
                    </template>

                    {{ uuidToName(ivod.userUuid) }}
                </v-tooltip>
            </div>
        </template>
    </generic-vod-picker>
</template>

<script lang="ts">

import Component, {mixins} from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { VodAssociation } from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import { TftMatchAccessibleVods } from '@client/js/squadov/vod'
import { WrappedTftMatch, TftParticipant } from '@client/js/tft/matches'
import TftLittleLegendIcon from '@client/vue/utility/tft/TftLittleLegendIcon.vue'
import GenericVodPicker from '@client/vue/utility/vods/GenericVodPicker.vue'
import { SquadOvGames } from '@client/js/squadov/game'
import CommonComponent from '@client/vue/CommonComponent'

@Component({
    components: {
        TftLittleLegendIcon,
        GenericVodPicker
    }
})
export default class TftVodPicker extends mixins(CommonComponent) {
    @Prop({required: true})
    matchUuid!: string

    @Prop({required: true})
    puuid!: string

    @Prop({required: true})
    vod!: VodAssociation | null

    @Prop({required: true})
    match!: WrappedTftMatch

    @Prop()
    timestamp!: Date | undefined | null

    @Prop({type: Boolean, default: false})
    disableFavorite!: boolean

    @Prop({type: Boolean, default: false})
    enableDraw!: boolean

    availableVods: TftMatchAccessibleVods | null = null

    get game(): SquadOvGames {
        return SquadOvGames.TeamfightTactics
    }

    get numPovs(): number {
        if (!this.availableVods) {
            return 0
        }
        return this.availableVods.vods.length
    }

    get uuidToParticipant(): (uuid: string) => TftParticipant | undefined {
        return  (uuid: string) => {
            let puuid = this.availableVods!.userMapping[uuid]!
            return this.match.data.info.participants.find((ele: TftParticipant) => ele.puuid === puuid)!
        }
    }

    get uuidToName(): (uuid: string) => string {
        return  (uuid: string) => {
            if (!this.availableVods) {
                return 'Loading...'
            }
            
            let puuid = this.availableVods!.userMapping[uuid]!
            return this.match.puuidToName[puuid]!
        }
    }

    @Watch('matchUuid')
    @Watch('isActive')
    refreshData() {
        if (!this.isActive) {
            return
        }
        this.availableVods = null
        apiClient.getTftMatchAccessibleVods(this.matchUuid).then((resp: ApiData<TftMatchAccessibleVods>) => {
            this.availableVods = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain TFT VODs: ', err)
        })
    }

    get uuidToVod(): Map<string, VodAssociation> {
        let map = new Map<string, VodAssociation>()
        if (!!this.availableVods) {
            for (let v of this.availableVods.vods) {
                map.set(v.userUuid, v)
            }
        }
        return map
    }

    get puuidToUuid(): Map<string, string> {
        let map = new Map<string, string>()
        if (!!this.availableVods) {
            for (let [uuid, puuid] of Object.entries(this.availableVods.userMapping)) {
                map.set(<string>puuid, <string>uuid)
            }
        }
        return map
    }

    allPovs: VodAssociation[] = []
    
    @Watch('selfPov')
    @Watch('nonSelfPovs')
    syncOptions() {
        this.allPovs = []
        if (!!this.selfPov) {
            this.allPovs.push(this.selfPov)
        }
        this.allPovs.push(...this.nonSelfPovs)
    }

    get selfPov(): VodAssociation | undefined {
        if (!this.availableVods) {
            return undefined
        }

        return this.availableVods.vods.find((ele: VodAssociation) => {
            let puuid = this.availableVods!.userMapping[ele.userUuid]
            return puuid === this.puuid
        })
    }

    get nonSelfPovs(): VodAssociation[] {
        if (!this.availableVods) {
            return []
        }
        return this.availableVods.vods.filter((ele: VodAssociation) => {
            let puuid = this.availableVods!.userMapping[ele.userUuid]
            return this.puuid !== puuid
        })
    }

    selectVod(vod: VodAssociation | null) {
        if (!!vod) {
            if (vod.videoUuid === this.vod?.videoUuid) {
                return
            }

            if (!this.availableVods) {
                return
            }
        }

        this.$emit('update:vod', vod)
    }

    selectPuuid(puuid: string) {
        if (!this.availableVods || this.numPovs == 0) {
            return
        }

        let uuid = this.puuidToUuid.get(puuid)
        if (!uuid) {
            console.log('Selected an invalid PUUID [uuid]: ', puuid, this.availableVods)
            return
        }

        let vod = this.uuidToVod.get(uuid)
        if (!vod) {
            console.log('Selected an invalid PUUID [vod]: ', puuid, this.availableVods)
            return
        }

        this.selectVod(vod)
    }

    @Watch('availableVods')
    onChangeVods() {
        this.$emit('update:vod', null)
        this.selectPuuid(this.puuid)
    }

    mounted() {
        this.refreshData()
    }
}

</script>

<style scoped>

.pov {
    border: 2px solid transparent !important;
}

.selected-pov {
    border: 2px solid var(--color-self) !important;
}

.selection-div {
    cursor: pointer;
}

</style>