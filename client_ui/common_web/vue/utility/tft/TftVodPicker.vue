<template>
    <div class="d-flex align-center pa-2" v-if="numPovs > 0">
        <div>
            <v-icon>
                mdi-eye
            </v-icon>
        </div>

        <v-divider vertical class="mx-2"></v-divider>

        <div class="d-flex">
            <template v-for="(pov, idx) in allPovs">
                <div
                    :class="`${(!!vod && pov.videoUuid === vod.videoUuid) ? 'selected-pov' : (!!selfPov && pov.videoUuid === selfPov.videoUuid) ? 'friendly-pov' : 'enemy-pov'} mx-4`"
                    :key="idx"
                >
                    <v-tooltip bottom>
                        <template v-slot:activator="{on, attrs}">
                            <div
                                v-on="on"
                                v-bind="attrs"
                            >
                                <tft-little-legend-icon
                                    :content-id="uuidToParticipant(pov.userUuid).companion.contentId"
                                    :width="48"
                                    :height="48"
                                >
                                </tft-little-legend-icon>
                            </div>
                        </template>

                        {{ uuidToName(pov.userUuid) }}
                    </v-tooltip>
                </div>
            </template>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { VodAssociation } from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import { TftMatchAccessibleVods } from '@client/js/squadov/vod'
import { WrappedTftMatch, TftParticipant } from '@client/js/tft/matches'
import TftLittleLegendIcon from '@client/vue/utility/tft/TftLittleLegendIcon.vue'

@Component({
    components: {
        TftLittleLegendIcon
    }
})
export default class LolVodPicker extends Vue {
    @Prop({required: true})
    matchUuid!: string

    @Prop({required: true})
    puuid!: string

    @Prop({required: true})
    vod!: VodAssociation | null

    @Prop({required: true})
    match!: WrappedTftMatch

    availableVods: TftMatchAccessibleVods | null = null

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
    refreshData() {
        this.availableVods = null
        apiClient.getTftMatchAccessibleVods(this.matchUuid, this.$store.state.currentUser.id).then((resp: ApiData<TftMatchAccessibleVods>) => {
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

    get allPovs(): VodAssociation[] {
        let ret: VodAssociation[] = []
        if (!!this.selfPov) {
            ret.push(this.selfPov)
        }
        ret.push(...this.nonSelfPovs)
        return ret
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

    selectVod(vod: VodAssociation) {
        if (vod.videoUuid === this.vod?.videoUuid) {
            return
        }

        if (!this.availableVods) {
            return
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

.enemy-pov {
    border: 2px solid rgb(255, 82, 82) !important;
}

.friendly-pov {
    border: 2px solid rgb(76, 175, 80) !important;
}

.selected-pov {
    border: 2px solid #FFD700 !important;
}

.selection-div {
    cursor: pointer;
}

</style>