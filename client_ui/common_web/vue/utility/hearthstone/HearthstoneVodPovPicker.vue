<template>
    <generic-vod-picker
        :value="vod"
        @input="selectVodForUserUuid(arguments[0].userUuid)"
        :options="orderedVods"
        :match-uuid="matchId"
        :timestamp="timestamp"
        :game="game"
        :disable-favorite="disableFavorite"
        :enable-draw="enableDraw"
        @update:enableDraw="$emit('update:enableDraw', arguments[0])"
    >
        <template v-slot:vod="{ivod, selected}">
            <hearthstone-hero-display
                :hero-card="uuidToHeroCard.get(ivod.userUuid)"
                :max-height="48"
                :class="`${(!!selected && ivod.videoUuid === selected.videoUuid) ? 'selected-hero' : (friendlyPovSet.has(ivod.videoUuid)) ? 'friendly-hero' : 'enemy-hero'} generic-hero`"
            >
            </hearthstone-hero-display> 
        </template>
    </generic-vod-picker>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { VodAssociation } from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import { HearthstoneMatchAccessibleVods } from '@client/js/squadov/vod'
import HearthstoneHeroDisplay from '@client/vue/utility/hearthstone/HearthstoneHeroDisplay.vue'
import GenericVodPicker from '@client/vue/utility/vods/GenericVodPicker.vue'
import { SquadOvGames } from '@client/js/squadov/game'

@Component({
    components: {
        HearthstoneHeroDisplay,
        GenericVodPicker,
    }
})
export default class HearthstoneVodPovPicker extends Vue {
    @Prop({required: true})
    matchId!: string

    @Prop({type: Number, required: true})
    refUserId!: number

    @Prop({required: true})
    vod!: VodAssociation | null

    @Prop()
    timestamp!: Date | undefined | null

    @Prop({type: Boolean, default: false})
    disableFavorite!: boolean

    @Prop({type: Boolean, default: false})
    enableDraw!: boolean

    availableVods: HearthstoneMatchAccessibleVods | null = null

    get game(): SquadOvGames {
        return SquadOvGames.Hearthstone
    }

    get orderedVods(): VodAssociation[] {
        return [...this.friendlyPovs, ...this.enemyPovs]
    }

    get friendlyPovs(): VodAssociation[] {
        if (!this.availableVods) {
            return []
        }
        return this.availableVods.vods.filter((ele: VodAssociation) => {
            return this.refUserId === this.availableVods!.userToId[ele.userUuid]
        })
    }

    get friendlyPovSet(): Set<string> {
        return new Set(this.friendlyPovs.map((ele: VodAssociation) => ele.videoUuid))
    }

    get enemyPovs(): VodAssociation[] {
        if (!this.availableVods) {
            return []
        }
        return this.availableVods.vods.filter((ele: VodAssociation) => {
            return this.refUserId !== this.availableVods!.userToId[ele.userUuid]
        })
    }

    get numPovs() : number {
        if (!this.availableVods) {
            return 0
        }
        return this.availableVods.vods.length
    }

    get uuidToVod() : Map<string, VodAssociation> {
        let map = new Map<string, VodAssociation>()
        if (!!this.availableVods) {
            for (let v of this.availableVods.vods) {
                map.set(v.userUuid, v)
            }
        }
        return map
    }

    get uuidToHeroCard() : Map<string, string> {
        let map = new Map<string, string>()
        if (!!this.availableVods) {
            for (let [userId, heroCard] of Object.entries(this.availableVods.userToHero)) {
                let uuid = this.userIdToUuid.get(parseInt(userId))
                if (!uuid) {
                    continue
                }
                map.set(uuid, <string>heroCard)
            }
        }
        return map
    }

    get userIdToUuid(): Map<number, string> {
        let map = new Map<number, string>()
        if (!!this.availableVods) {
            for (let [uuid, userId] of Object.entries(this.availableVods.userToId)) {
                map.set(<number>userId, <string>uuid)
            }
        }
        return map
    }

    @Watch('userIdToUuid')
    @Watch('availableVods')
    onChangeVods() {
        this.$emit('update:vod', null)
        if (!this.availableVods) {
            console.log('no vods')
            return
        }

        // By default we assume we want to watch the VOD of whoever's match we're looking at
        // which can't be assumed to be the local user.
        let uuid = this.userIdToUuid.get(this.refUserId)
        if (!uuid) {
            return
        }

        this.selectVodForUserUuid(uuid)
    }

    selectVodForUserUuid(uuid: string) {
        if (!this.availableVods) {
            return
        }

        let vod = this.uuidToVod.get(uuid)
        if (!vod) {
            return
        }

        this.$emit('update:vod', vod)
    }

    @Watch('matchId')
    refreshData() {
        this.availableVods = null
        apiClient.getHearthstoneMatchAccessibleVods(this.matchId).then((resp: ApiData<HearthstoneMatchAccessibleVods>) => {
            this.availableVods = resp.data
        }).catch((err: any) => {
            console.log('Failed to get accessible Hearthstone match VODS: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>


<style scoped>

.generic-hero {
    border-radius: 4px;
}

.friendly-hero {
    border: 2px solid rgb(76, 175, 80) !important;
}

.enemy-hero {
    border: 2px solid rgb(255, 82, 82) !important;
}

.selected-hero {
    border: 2px solid #FFD700 !important;
}

.selection-div {
    cursor: pointer;
}

</style>