<template>
    <div class="d-flex align-center pa-2">
        <div>
            <v-icon>
                mdi-eye
            </v-icon>
        </div>

        <v-divider vertical class="mx-2"></v-divider>

        <div class="d-flex">
            <div
                class="selection-div mr-2"
                v-for="(pvod, idx) in allPovs"
                :key="`pov-${idx}`"
                @click="selectVod(pvod)"
            >
                <v-tooltip offset-x right>
                    <template v-slot:activator="{on, attrs}">
                        <div
                            v-bind="attrs"
                            v-on="on"
                            class="ma-1"
                        >
                            <wow-class-spec-icon
                                :spec-id="charForUuid(pvod.userUuid).specId"
                                :class="(!!vod && vod.videoUuid === pvod.videoUuid) ? 'selected-char' : 'unselected-char'"
                            >
                            </wow-class-spec-icon>    
                        </div>
                    </template>

                    <wow-character-display
                        :character="charForUuid(pvod.userUuid)"
                    >
                    </wow-character-display>    
                </v-tooltip>
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import {
    WowCharacter,
    WoWCharacterUserAssociation
} from '@client/js/wow/character'
import { WowMatchAccessibleVods, VodAssociation } from '@client/js/squadov/vod'
import { apiClient, ApiData } from '@client/js/api'
import WowClassSpecIcon from '@client/vue/utility/wow/WowClassSpecIcon.vue'
import WowCharacterDisplay from '@client/vue/utility/wow/WowCharacterDisplay.vue'

@Component({
    components: {
        WowClassSpecIcon,
        WowCharacterDisplay
    }
})
export default class WowVodPovPicker extends Vue {
    @Prop({required: true})
    vod!: VodAssociation | null

    @Prop({required: true})
    matchUuid!: string

    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    matchCharacters!: WowCharacter[]

    @Prop({required: true})
    characterAssociations!: WoWCharacterUserAssociation[]

    allAccessibleVods: WowMatchAccessibleVods | null = null

    get uuidToVod(): Map<string, VodAssociation> {
        let map = new Map<string, VodAssociation>()
        if (!!this.allAccessibleVods) {
            for (let v of this.allAccessibleVods.vods) {
                map.set(v.userUuid, v)
            }
        }
        return map
    }

    get userIdToUuid(): Map<number, string> {
        let map = new Map<number, string>()
        if (!!this.allAccessibleVods) {
            for (let [uuid, userId] of Object.entries(this.allAccessibleVods.userToId)) {
                map.set(<number>userId, <string>uuid)
            }
        }
        return map
    }

    get charToObject(): Map<string, WowCharacter> {
        let ret = new Map()
        for (let c of this.matchCharacters) {
            ret.set(c.guid, c)
        }
        return ret
    }

    get userIdToChar(): Map<number, WowCharacter> {
        let ret = new Map()
        for (let assoc of this.characterAssociations) {
            let char = this.charToObject.get(assoc.guid)
            if (!char) {
                continue
            }
            
            ret.set(assoc.userId, char)
        }
        return ret
    }

    get allPovs(): VodAssociation[] {
        if (!this.allAccessibleVods) {
            return []
        }
        return this.allAccessibleVods.vods.filter((ele: VodAssociation) => {
            return !!this.charForUuid(ele.userUuid)
        })
    }

    charForUuid(uuid: string): WowCharacter | null {
        if (!this.allAccessibleVods) {
            return null
        }

        let uid = this.allAccessibleVods.userToId[uuid]
        if (!uid) {
            return null
        }

        let char = this.userIdToChar.get(uid)
        if (!char) {
            return null
        }

        return char
    }

    @Watch('allAccessibleVods')
    resetVods() {
        this.$emit('update:vod', null)
        this.selectUserId(this.userId)
    }

    refreshData() {
        apiClient.getWoWMatchAccessibleVods(this.$store.state.currentUser.id, this.matchUuid).then((resp: ApiData<WowMatchAccessibleVods>) => {
            this.allAccessibleVods = resp.data
        }).catch((err: any) => {
            console.log('Failed to get WoW match accessible VODs: ', err)
        })
    }

    selectUserId(userId: number) {
        let uuid = this.userIdToUuid.get(userId)
        if (!uuid) {
            console.log(`Selected Invalid WoW Match User ID [UUID]: ${userId}`)
            return
        }

        let vod = this.uuidToVod.get(uuid)
        if (!vod) {
            console.log(`Selected Invalid WoW Match User ID [VOD]: ${userId}`)
            return
        }
        this.selectVod(vod)
    }

    selectVod(vod: VodAssociation) {
        if (vod.videoUuid === this.vod?.videoUuid) {
            return
        }

        if (!this.allAccessibleVods) {
            return
        }

        this.$emit('update:vod', vod)
    }

    mounted() {
        this.refreshData()
    }
}

</script>

<style scoped>

.selection-div {
    cursor: pointer;
}

.selected-char {
    border: 2px solid #FFD700 !important;
}

.unselected-char {
    border: 2px solid transparent !important;
}

</style>