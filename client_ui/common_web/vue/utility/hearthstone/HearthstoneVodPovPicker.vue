<template>
    <div class="d-flex align-center pa-2" v-if="numPovs > 0">
        <v-icon>
            mdi-eye
        </v-icon>

        <v-divider vertical class="mx-2"></v-divider>

        <div class="d-flex">
            <div
                class="selection-div mr-2"
                v-for="(fvod, fidx) in friendlyPovs"
                :key="`friendly-${fidx}`"
                @click="selectVodForUserUuid(fvod.userUuid)"
            >
                <hearthstone-hero-display
                    :hero-card="uuidToHeroCard.get(fvod.userUuid)"
                    :max-height="48"
                    :class="`${(!!vod && fvod.videoUuid === vod.videoUuid) ? 'selected-hero' : 'friendly-hero'} generic-hero`"
                >
                </hearthstone-hero-display> 
            </div>

            <div
                class="selection-div mr-2"
                v-for="(evod, eidx) in enemyPovs"
                :key="`enemy-${eidx}`"
                @click="selectVodForUserUuid(evod.userUuid)"
            >
                <hearthstone-hero-display
                    :hero-card="uuidToHeroCard.get(evod.userUuid)"
                    :max-height="48"
                    :class="`${(!!vod && evod.videoUuid === vod.videoUuid) ? 'selected-hero' : 'enemy-hero'} generic-hero`"
                >
                </hearthstone-hero-display>
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
import { HearthstoneMatchAccessibleVods } from '@client/js/squadov/vod'
import { getActiveUserId } from '@client/js/app'
import HearthstoneHeroDisplay from '@client/vue/utility/hearthstone/HearthstoneHeroDisplay.vue'

@Component({
    components: {
        HearthstoneHeroDisplay
    }
})
export default class HearthstoneVodPovPicker extends Vue {
    @Prop({required: true})
    matchId!: string

    @Prop({type: Number, required: true})
    refUserId!: number

    @Prop({required: true})
    vod!: VodAssociation | null

    availableVods: HearthstoneMatchAccessibleVods | null = null

    get friendlyPovs(): VodAssociation[] {
        if (!this.availableVods) {
            return []
        }
        return this.availableVods.vods.filter((ele: VodAssociation) => {
            return this.refUserId === this.availableVods!.userToId[ele.userUuid]
        })
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
        apiClient.getHearthstoneMatchAccessibleVods(this.matchId, getActiveUserId()).then((resp: ApiData<HearthstoneMatchAccessibleVods>) => {
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