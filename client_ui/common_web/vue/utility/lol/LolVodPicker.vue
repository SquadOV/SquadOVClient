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
                <lol-participant-display
                    :participant="participantFromUserUuid(fvod.userUuid)"
                    :current-participant-id="currentParticipantId"
                    :match="match"
                    :width-height="48"
                    no-border
                    :class="(!!vod && fvod.videoUuid === vod.videoUuid) ? 'selected-pov' : 'friendly-pov'"
                >
                </lol-participant-display>
            </div>

            <div
                class="selection-div mr-2"
                v-for="(evod, eidx) in enemyPovs"
                :key="`enemy-${eidx}`"
                @click="selectVod(evod)"
            >
                <lol-participant-display
                    :participant="participantFromUserUuid(evod.userUuid)"
                    :current-participant-id="currentParticipantId"
                    :match="match"
                    :width-height="48"
                    no-border
                    :class="(!!vod && evod.videoUuid === vod.videoUuid) ? 'selected-pov' : 'enemy-pov'"
                >
                </lol-participant-display>
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
import { LeagueMatchAccessibleVods } from '@client/js/squadov/vod'
import {
    LolMatch,
    extractSameTeamPlayersFromParticipantId,
    extractEnemyTeamPlayersFromParticipantId,
    extractSameTeamPlayersFromTeamId,
    getPlayerFromParticipantId,
} from '@client/js/lol/matches'
import { WrappedLolParticipant } from '@client/js/lol/participant'
import LolParticipantDisplay from '@client/vue/utility/lol/LolParticipantDisplay.vue'

@Component({
    components: {
        LolParticipantDisplay
    }
})
export default class LolVodPicker extends Vue {
    @Prop({required: true})
    matchUuid!: string

    @Prop({required: true})
    currentParticipantId!: number | undefined | null

    @Prop({required: true})
    match!: LolMatch

    @Prop({required: true})
    vod!: VodAssociation | null

    availableVods: LeagueMatchAccessibleVods | null = null

    get numPovs(): number {
        if (!this.availableVods) {
            return 0
        }
        return this.availableVods.vods.length
    }

    @Watch('matchUuid')
    refreshData() {
        this.availableVods = null
        apiClient.getLolMatchAccessibleVods(this.matchUuid, this.$store.state.currentUser.id).then((resp: ApiData<LeagueMatchAccessibleVods>) => {
            this.availableVods = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain LoL VODs: ', err)
        })
    }

    participantFromUserUuid(userUuid: string): WrappedLolParticipant | undefined {
        if (!this.availableVods) {
            return undefined
        }

        let participantId = this.availableVods.userMapping[userUuid]
        if (!participantId) {
            return undefined
        }

        return getPlayerFromParticipantId(this.match, participantId)
    }

    get participantIdToUserUuid(): Map<number, string> {
        let ret = new Map()
        if (!!this.availableVods) {
            for (let [uuid, participantId] of Object.entries(this.availableVods.userMapping)) {
                ret.set(<number>participantId, <string>uuid)
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

    get friendlyParticipants(): WrappedLolParticipant[] {
        if (!!this.currentParticipantId) {
            return extractSameTeamPlayersFromParticipantId(this.match, this.currentParticipantId)!
        } else {
            return extractSameTeamPlayersFromTeamId(this.match, 100)
        }
    }

    get enemyParticipants(): WrappedLolParticipant[] {
        if (!!this.currentParticipantId) {
            return extractEnemyTeamPlayersFromParticipantId(this.match, this.currentParticipantId)!
        } else {
            return extractSameTeamPlayersFromTeamId(this.match, 200)
        }
    }

    get friendlyPovs(): VodAssociation[] {
        return <VodAssociation[]>this.friendlyParticipants.map((ele: WrappedLolParticipant) => {
            let userUuid = this.participantIdToUserUuid.get(ele.participant.participantId)
            if (!userUuid) {
                return undefined
            }

            return this.uuidToVod.get(userUuid)
        }).filter((ele: VodAssociation | undefined) => {
            return !!ele
        })
    }

    get enemyPovs(): VodAssociation[] {
        return <VodAssociation[]>this.enemyParticipants.map((ele: WrappedLolParticipant) => {
            let userUuid = this.participantIdToUserUuid.get(ele.participant.participantId)
            if (!userUuid) {
                return undefined
            }

            return this.uuidToVod.get(userUuid)
        }).filter((ele: VodAssociation | undefined) => {
            return !!ele
        })
    }

    selectParticipantId(participantId: number | undefined | null) {
        if (!participantId) {
            // In this case we want to select any available VOD.
            if (this.numPovs > 0) {
                this.selectVod(this.availableVods!.vods[0])
            }
            return
        }

        // In other failure cases, we want to make sure the state doesn't change.
        let userUuid = this.participantIdToUserUuid.get(participantId)
        if (!userUuid) {
            console.log('Cannot select participant id [no user uuid]: ', participantId)
            return
        }

        let vod = this.uuidToVod.get(userUuid)
        if (!vod) {
            console.log('Cannot select participant id [no vod]: ', participantId)
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
        this.selectParticipantId(this.currentParticipantId)
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