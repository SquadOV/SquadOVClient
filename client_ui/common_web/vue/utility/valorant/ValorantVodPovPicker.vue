<template>
    <div class="d-flex align-center pa-2" v-if="numPovs > 0">
        <div>
            <v-icon>
                mdi-eye
            </v-icon>
        </div>

        <v-divider vertical class="mx-2"></v-divider>

        <div class="d-flex">
            <div
                class="selection-div mr-2"
                v-for="(fvod, fidx) in friendlyPovs"
                :key="`friendly-${fidx}`"
                @click="selectVod(fvod)"
            >
                <valorant-agent-icon
                    :agent="agentForPlayer(fvod.userUuid)"
                    :width-height="48"
                    :class="(!!vod && fvod.videoUuid === vod.videoUuid) ? 'selected-agent' : 'friendly-agent'"
                    circular
                >
                </valorant-agent-icon>
            </div>

            <div
                class="selection-div mr-2"
                v-for="(evod, eidx) in enemyPovs"
                :key="`enemy-${eidx}`"
                @click="selectVod(evod)"
            >
                <valorant-agent-icon
                    :agent="agentForPlayer(evod.userUuid)"
                    :width-height="48"
                    :class="(!!vod && evod.videoUuid === vod.videoUuid) ? 'selected-agent' : 'enemy-agent'"
                    circular
                >
                </valorant-agent-icon>
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { VodAssociation } from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import { ValorantMatchPlayerMatchMetadata } from '@client/js/valorant/valorant_matches'
import { ValorantMatchDetailsWrapper, ValorantMatchTeamWrapper } from '@client/js/valorant/valorant_matches_parsed'
import { ValorantMatchAccessibleVods } from '@client/js/squadov/vod'
import { getActiveUserId } from '@client/js/app'

import ValorantAgentIcon from '@client/vue/utility/valorant/ValorantAgentIcon.vue'

@Component({
    components: {
        ValorantAgentIcon
    }
})
export default class ValorantVodPovPicker extends Vue {
    @Prop({required: true})
    puuid!: string

    // Which user we're considering to be "on our team"
    @Prop({required: true})
    refPuuid!: string

    @Prop({required: true})
    matchUuid!: string

    @Prop({required: true})
    vod!: VodAssociation | null

    @Prop({required: true})
    playerMetadata!: ValorantMatchPlayerMatchMetadata | null

    @Prop({required: true})
    match!: ValorantMatchDetailsWrapper | null

    availableVods: ValorantMatchAccessibleVods | null = null

    agentForPlayer(uuid: string) : string {
        if (!this.match || !this.availableVods) {
            return ''
        }

        let puuid = this.availableVods.userMapping[uuid]
        if (!puuid) {
            return ''
        }
        return this.match.getPlayerAgentId(puuid)
    }

    get friendlyPovs(): VodAssociation[] {
        if (!this.match) {
            return []
        }
        return this.getTeamPovs(this.match.getPlayerTeam(this.refPuuid))
    }

    get enemyPovs(): VodAssociation[] {
        if (!this.match) {
            return []
        }

        // Special case for deathmatch where enemy POVs is just everyone
        // besides the current player (refPuuid).
        if (this.match.isDeathmatch) {
            return this.getNonSelfPovs()
        } else {
            return this.getTeamPovs(this.match.getOpposingPlayerTeam(this.refPuuid))
        }
    }

    getNonSelfPovs(): VodAssociation[] {
        if (!this.availableVods) {
            return []
        }
        return this.availableVods.vods.filter((ele: VodAssociation) => {
            let puuid = this.availableVods!.userMapping[ele.userUuid]
            return this.refPuuid !== puuid
        })
    }

    getTeamPovs(team: ValorantMatchTeamWrapper) : VodAssociation[] {
        if (!this.availableVods) {
            return []
        }
        return this.availableVods.vods.filter((ele: VodAssociation) => {
            let puuid = this.availableVods!.userMapping[ele.userUuid]
            if (!puuid) {
                return false
            }
            return team.hasPlayer(puuid)
        })
    }

    get numPovs(): number {
        if (!this.availableVods) {
            return 0
        }
        return this.availableVods.vods.length
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

    refreshData() {
        this.availableVods = null
        if (!this.match) {
            return
        }

        // Using the current user ID here is correct as we only want the VODS accessible to the current user (not to the user whose match we're viewing).
        apiClient.getValorantMatchAccessibleVods(this.matchUuid, getActiveUserId()).then((resp: ApiData<ValorantMatchAccessibleVods>) => {
            this.availableVods = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain Valorant VODs: ', err)
        })
    }

    @Watch('availableVods')
    onChangeVods() {
        this.$emit('update:vod', null)
        this.$emit('update:playerMetadata', null)
        this.selectPuuid(this.puuid)
    }

    selectVod(vod: VodAssociation) {
        if (vod.videoUuid === this.vod?.videoUuid) {
            return
        }

        if (!this.availableVods) {
            return
        }

        let puuid = this.availableVods!.userMapping[vod.userUuid]
        if (!puuid) {
            return
        }

        this.$emit('update:puuid', puuid)
        this.$emit('update:vod', vod)
        apiClient.getValorantMatchPlayerMetadata(this.matchUuid, puuid).then((resp : ApiData<ValorantMatchPlayerMatchMetadata>) => {
            this.$emit('update:playerMetadata', resp.data)
        }).catch((err : any) => {
            console.log('Failed to obtain Valorant VOD Player Metadata: ', err)
        })
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

    mounted() {
        this.refreshData()
    }
}

</script>

<style scoped>

.friendly-agent {
    border: 2px solid rgb(76, 175, 80) !important;
}

.enemy-agent {
    border: 2px solid rgb(255, 82, 82) !important;
}

.selected-agent {
    border: 2px solid #FFD700 !important;
}

.selection-div {
    cursor: pointer;
}

</style>