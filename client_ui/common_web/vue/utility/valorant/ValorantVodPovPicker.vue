<template>
    <generic-vod-picker
        :value="vod"
        @input="selectVod"
        :options="orderedVods"
        :match-uuid="matchUuid"
        :timestamp="timestamp"
        :game="game"
        :disable-favorite="disableFavorite"
        :enable-draw="enableDraw"
        @update:enableDraw="$emit('update:enableDraw', arguments[0])"
    >
        <template v-slot:vod="{ivod, selected}">
            <valorant-agent-icon
                :agent="agentForPlayer(ivod.userUuid)"
                :width-height="48"
                :class="(!!selected && ivod.videoUuid === selected.videoUuid) ? 'selected-agent' : (friendlyPovSet.has(ivod.videoUuid)) ? 'friendly-agent' : 'enemy-agent'"
                circular
            >
            </valorant-agent-icon>
        </template>
    </generic-vod-picker>
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

import ValorantAgentIcon from '@client/vue/utility/valorant/ValorantAgentIcon.vue'
import GenericVodPicker from '@client/vue/utility/vods/GenericVodPicker.vue'
import { SquadOvGames } from '@client/js/squadov/game'

@Component({
    components: {
        GenericVodPicker,
        ValorantAgentIcon,
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

    @Prop()
    timestamp!: Date | undefined | null

    @Prop({type: Boolean, default: false})
    disableFavorite!: boolean

    @Prop({type: Boolean, default: false})
    enableDraw!: boolean

    availableVods: ValorantMatchAccessibleVods | null = null

    get game(): SquadOvGames {
        return SquadOvGames.Valorant
    }

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

    get orderedVods(): VodAssociation[] {
        return [...this.friendlyPovs, ...this.enemyPovs]
    }

    get friendlyPovs(): VodAssociation[] {
        if (!this.match) {
            return []
        }
        return this.getTeamPovs(this.match.getPlayerTeam(this.refPuuid))
    }
    
    get friendlyPovSet(): Set<string> {
        return new Set(this.friendlyPovs.map((ele: VodAssociation) => ele.videoUuid))
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
        apiClient.getValorantMatchAccessibleVods(this.matchUuid).then((resp: ApiData<ValorantMatchAccessibleVods>) => {
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
    border: 2px solid var(--color-friendly) !important;
}

.enemy-agent {
    border: 2px solid var(--color-enemy) !important;
}

.selected-agent {
    border: 2px solid var(--color-self) !important;
}

.selection-div {
    cursor: pointer;
}

</style>