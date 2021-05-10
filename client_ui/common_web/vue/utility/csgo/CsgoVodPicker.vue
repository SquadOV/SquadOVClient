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
            <steam-account-display
                :width="48"
                :height="48"
                :account="steamAccountFromUserUuid(ivod.userUuid)"
                :class="(!!selected && ivod.videoUuid === selected.videoUuid) ? 'selected-pov' : (friendlyPovSet.has(ivod.videoUuid)) ? 'friendly-pov' : 'enemy-pov'"
            >
            </steam-account-display>
        </template>
    </generic-vod-picker>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { VodAssociation, CsgoMatchAccessibleVods } from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import { CsgoFullMatchDataWrapper } from '@client/js/csgo/match'
import { SquadOvGames } from '@client/js/squadov/game'
import { getActiveUserId } from '@client/js/app'

import GenericVodPicker from '@client/vue/utility/vods/GenericVodPicker.vue'
import SteamAccountDisplay from '@client/vue/utility/steam/SteamAccountDisplay.vue'
import { SteamAccount } from '@client/js/steam/account'

@Component({
    components: {
        GenericVodPicker,
        SteamAccountDisplay,
    }
})
export default class CsgoVodPicker extends Vue {
    @Prop({required: true})
    matchUuid!: string

    @Prop({required: true})
    currentSteamId!: number | undefined | null

    @Prop({required: true})
    match!: CsgoFullMatchDataWrapper

    @Prop({required: true})
    vod!: VodAssociation | null

    @Prop()
    timestamp!: Date | undefined | null

    @Prop({type: Boolean, default: false})
    disableFavorite!: boolean

    @Prop({type: Boolean, default: false})
    enableDraw!: boolean

    availableVods: CsgoMatchAccessibleVods | null = null

    get game(): SquadOvGames {
        return SquadOvGames.Csgo
    }

    get numPovs(): number {
        if (!this.availableVods) {
            return 0
        }
        return this.availableVods.vods.length
    }

    get steamIdToUserUuid(): Map<number, string> {
        let ret = new Map()
        if (!!this.availableVods) {
            for (let [uuid, steamId] of Object.entries(this.availableVods.userMapping)) {
                ret.set(<number>steamId, <string>uuid)
            }
        }
        return ret
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

    get orderedVods(): VodAssociation[] {
        return [...this.friendlyPovs, ...this.enemyPovs]
    }

    get friendlyPovs(): VodAssociation[] {
        if (!this.currentSteamId) {
            return []
        }

        let steamIds = this.match.friendlySteamIds(this.currentSteamId)
        return <VodAssociation[]>steamIds.map((id: number) => {
            let userUuid = this.steamIdToUserUuid.get(id)
            if (!userUuid) {
                return null
            }

            let vod = this.uuidToVod.get(userUuid)
            if (!vod) {
                return null
            }
            return vod
        }).filter((ele: VodAssociation | null) => !!ele)
    }

    get enemyPovs(): VodAssociation[] {
        if (!this.currentSteamId) {
            return []
        }

        let steamIds = this.match.enemySteamIds(this.currentSteamId)
        return <VodAssociation[]>steamIds.map((id: number) => {
            let userUuid = this.steamIdToUserUuid.get(id)
            if (!userUuid) {
                return null
            }

            let vod = this.uuidToVod.get(userUuid)
            if (!vod) {
                return null
            }
            return vod
        }).filter((ele: VodAssociation | null) => !!ele)
    }

    @Watch('matchUuid')
    refreshData() {
        this.availableVods = null
        apiClient.getCsgoMatchAccessibleVods(this.matchUuid, getActiveUserId()).then((resp: ApiData<CsgoMatchAccessibleVods>) => {
            this.availableVods = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain CSGO VODs: ', err)
        })
    }

    steamAccountFromUserUuid(uuid: string): SteamAccount | undefined {
        if (!this.availableVods) {
            return undefined
        }
        let steamId = this.availableVods.userMapping[uuid]
        if (steamId === undefined) {
            return undefined
        }
        return this.match.steamAccount(steamId)
    }

    selectSteamId(steamId: number | undefined | null) {
        if (!steamId) {
            // In this case we want to select any available VOD.
            if (this.numPovs > 0) {
                this.selectVod(this.availableVods!.vods[0])
            }
            return
        }

        // In other failure cases, we want to make sure the state doesn't change.
        let userUuid = this.steamIdToUserUuid.get(steamId)
        if (!userUuid) {
            console.log('Cannot select steam id [no user uuid]: ', steamId)
            return
        }

        let vod = this.uuidToVod.get(userUuid)
        if (!vod) {
            console.log('Cannot select steam id [no vod]: ', steamId)
            return
        }
        this.selectVod(vod)
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

    @Watch('availableVods')
    onChangeVods() {
        this.$emit('update:vod', null)
        this.selectSteamId(this.currentSteamId)
    }

    mounted() {
        this.refreshData()
    }
}
</script>

<style scoped>

.friendly-pov {
    border: 2px solid rgb(76, 175, 80) !important;
}

.enemy-pov {
    border: 2px solid rgb(255, 82, 82) !important;
}

.selected-pov {
    border: 2px solid #FFD700 !important;
}

.selection-div {
    cursor: pointer;
}

</style>