<template>
    <loading-container :is-loading="internalValue === null">
        <template v-slot:default="{ loading }">
            <v-tabs grow v-if="!loading">
                <v-tab>
                    Squads
                </v-tab>

                <v-tab-item>
                    <loading-container :is-loading="validSquads === null">
                        <template v-slot:default="{ loading }">
                            <div v-if="!loading">
                                <v-data-table
                                    :headers="squadConnHeaders"
                                    :items="squadConnItems"
                                    :items-per-page="5"
                                >
                                    <template v-slot:item.squad="{ item }">
                                        <span class="font-weight-bold" v-if="!!item.squad">
                                            {{ item.squad.squadName }} ({{ item.squad.squadGroup }})
                                        </span>
                                    </template>

                                    <template v-slot:item.share="{ item }">
                                        <v-checkbox
                                            class="ma-0"
                                            :input-value="item.share"
                                            dense
                                            hide-details
                                            v-if="!squadEditsPending[item.squad.id]"
                                            @change="onEditSquadConnection(item.squad.id, arguments[0], item.clip)"
                                        >
                                        </v-checkbox>

                                        <v-progress-circular size="16" indeterminate v-else>
                                        </v-progress-circular>
                                    </template>

                                    <template v-slot:item.clip="{ item }">
                                        <v-checkbox
                                            class="ma-0"
                                            :input-value="item.clip"
                                            dense
                                            hide-details
                                            v-if="!squadEditsPending[item.squad.id]"
                                            @change="onEditSquadConnection(item.squad.id, item.share, arguments[0])"
                                        >
                                        </v-checkbox>

                                        <v-progress-circular size="16" indeterminate v-else>
                                        </v-progress-circular>
                                    </template>

                                    <template v-slot:item.actions="{ item }">
                                        <v-btn
                                            small
                                            icon
                                            color="error"
                                            :loading="deletePending"
                                            @click="deleteConnectionsFromSquad(item.squad.id)"
                                        >
                                            <v-icon>
                                                mdi-close-circle
                                            </v-icon>
                                        </v-btn>
                                    </template>
                                </v-data-table>

                                <div class="d-flex align-center">
                                    <div class="text-overline">
                                        New:
                                    </div>

                                    <v-select
                                        class="mx-2"
                                        label="Squads"
                                        v-model="squadsToShare"
                                        :items="shareSquadItems"
                                        deletable-chips
                                        chips
                                        multiple
                                        clearable
                                        outlined
                                        hide-details
                                        dense
                                    >
                                    </v-select>

                                    <v-btn
                                        color="primary"
                                        :loading="addPending"
                                        @click="shareWithNewSquads"
                                    >
                                        Share
                                    </v-btn>
                                </div>
                            </div>
                        </template>
                    </loading-container>
                </v-tab-item>
            </v-tabs>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { Prop, Watch } from 'vue-property-decorator'
import { MatchVideoShareConnection } from '@client/js/squadov/share'
import { Squad, SquadMembership } from '@client/js/squadov/squad'
import { apiClient, ApiData } from '@client/js/api'
import { SquadOvGames } from '@client/js/squadov/game'

// So this component needs to display the user's share settings to
// individual users (tbd), squads, and communities (when that's ready).
// We'll display each group of those share connections in a tab.
//
// Note that each connection has a singular destination (user, squad, or community)
// and we can be sharing:
//
//  1) A match
//  2) A video
//  3) Both.
//
// Which leaves us in a bit of a problem because in the case of sharing a match,
// we could have MULTIPLE connections sharing the same match as we generally
// share the (match, video) tuple. For simplicity we'll always assume that
// every (match, video) share connection is shared in the exact same way! Thus,
// when we change a singular share connection for a particular match, we need to change
// every connection that exists. Similarly, when add a new connection, we must add
// a connection for every VOD.
//
// The singular VOD/match cases are trivial.
@Component({
    components: {
        LoadingContainer
    }
})
export default class ShareConnectionsEditor extends Vue {
    // Only one of matchUuid and videoUuid should ever be set.
    // This is used to determine which endpoint for modifications
    // we should use.
    @Prop()
    matchUuid!: string | undefined

    @Prop()
    videoUuid!: string | undefined

    @Prop()
    game!: SquadOvGames | undefined

    @Prop({type: Boolean, default: false})
    noClip!: boolean

    internalValue: MatchVideoShareConnection[] | null = []
    validSquads: Squad[] | null = null
    squadsToShare: Squad[] = []
    
    addPending: boolean = false
    deletePending: boolean = false
    squadEditsPending: { [squadId: number] : boolean | undefined } = {}

    get shareSquadItems(): any[] {
        if (!this.validSquads) {
            return []
        }
        return this.validSquads
            .filter((ele: Squad) => !this.groupedBySquad.has(ele.id))
            .map((ele: Squad) => {
                return {
                    text: `${ele.squadName} (${ele.squadGroup})`,
                    value: ele,
                }
            })
    }

    get groupedBySquad(): Map<number, MatchVideoShareConnection[]> {
        let ret: Map<number, MatchVideoShareConnection[]> = new Map()
        if (!!this.internalValue) {
            for (let v of this.internalValue) {
                if (v.destSquadId !== null) {
                    if (!ret.has(v.destSquadId)) {
                        ret.set(v.destSquadId, [])
                    }
                    ret.get(v.destSquadId)!.push(v)
                }
            }
        }
        return ret
    }
    
    get squadConnHeaders(): any[] {
        let ret: any[] = [
            {
                text: 'Squad',
                value: 'squad',
            },
            {
                text: 'Share',
                value: 'share',
            },
        ]

        if (!this.noClip) {
            ret.push({
                text: 'Clip',
                value: 'clip',
            })
        }

        ret.push({
            text: 'Actions',
            value: 'actions',
            sortable: false,
            filterable: false
        })
        return ret
    }

    get squadMap(): Map<number, Squad> {
        if (!this.validSquads) {
            return new Map()
        }

        let ret = new Map()
        for (let s of this.validSquads) {
            ret.set(s.id, s)
        }
        return ret
    }

    get squadConnItems(): any[] {
        let validSquads = Array.from(this.groupedBySquad.keys())
        
        return validSquads.map((id: number) => {
            let conns = this.groupedBySquad.get(id)!
            return {
                squad: this.squadMap.get(id)!,
                share: conns.some((ele: MatchVideoShareConnection) => ele.canShare),
                clip: conns.some((ele: MatchVideoShareConnection) => ele.canClip),
            }
        })
    }

    @Watch('matchUuid')
    @Watch('videoUuid')
    refreshConnections() {
        this.internalValue = null

        let promise = !!this.matchUuid ? apiClient.getMatchShareConnections(this.matchUuid) : apiClient.getClipShareConnections(this.videoUuid!)
        promise.then((resp: ApiData<MatchVideoShareConnection[]>) => {
            this.internalValue = resp.data
        }).catch((err: any) => {
            console.log('Failed to get share connections: ', err)
        })
    }

    refreshSquads() {
        apiClient.getUserSquads(this.$store.state.currentUser.id).then((resp: ApiData<SquadMembership[]>) => {
            this.validSquads = resp.data
                .map((ele: SquadMembership) => ele.squad)
                .sort((a: Squad, b: Squad) => {
                    if (a.squadName < b.squadName) {
                        return -1
                    } else if (a.squadName > b.squadName) {
                        return 1
                    }
                    return 0
                })
        }).catch((err : any) => {
            console.log('Failed to get user squads [share connections editor]: ', err)
        })
    }
    
    mergeIncomingConnections(conns: MatchVideoShareConnection[]) {
        if (!this.internalValue) {
            return
        }

        for (let c of conns) {
            let idx = this.internalValue.findIndex((ele: MatchVideoShareConnection) => ele.id === c.id)
            if (idx === -1) {
                this.internalValue.push(c)
            } else {
                Vue.set(this.internalValue, idx, c)
            }
        }
    }

    onEditSquadConnection(squadId: number, canShare: boolean, canClip: boolean) {
        let conns = this.groupedBySquad.get(squadId)!
        Vue.set(this.squadEditsPending, squadId, true)

        Promise.all(conns.map((ele: MatchVideoShareConnection) => {
            return apiClient.editShareConnection(ele.id, canShare, canClip)
        })).then(() => {
            conns.forEach((ele: MatchVideoShareConnection) => {
                ele.canShare = canShare
                ele.canClip = canClip
            })
        }).catch((err: any) => {
            console.log('Failed to edit squad share connections: ', err)
        }).finally(() => {
            Vue.set(this.squadEditsPending, squadId, false)
        })
    }

    shareWithNewSquads() {
        this.addPending = true
        Promise.all(this.squadsToShare.map((ele: Squad) => {
            return apiClient.createShareConnection({
                id: -1,
                matchUuid: !!this.matchUuid ? this.matchUuid : null,
                videoUuid: !!this.videoUuid ? this.videoUuid : null,
                destUserId: null,
                destSquadId: ele.id,
                canShare: true,
                canClip: true,
            }, this.game)
        })).then((resp: ApiData<MatchVideoShareConnection[]>[]) => {
            // Add the new (or possibly edited) connections to the internal value store.
            this.mergeIncomingConnections(resp.map((ele: ApiData<MatchVideoShareConnection[]>) => ele.data).flat())
            this.squadsToShare = []
        }).catch((err: any) => {
            console.log('Failed to share with squads: ', err)
        }).finally(() => {
            this.addPending = false
        })
    }

    deleteConnectionsFromSquad(squadId: number) {
        if (!this.groupedBySquad.has(squadId)) {
            return
        }
        this.deletePending = true

        let conns = this.groupedBySquad.get(squadId)!
        Promise.all(conns.map((ele: MatchVideoShareConnection) => {
            return apiClient.deleteShareConnection(ele.id)
        })).then(() => {
            if (!this.internalValue) {
                return
            }

            for (let c of conns) {
                let idx = this.internalValue.findIndex((ele: MatchVideoShareConnection) => ele.id === c.id)
                if (idx !== -1) {
                    this.internalValue.splice(idx, 1)
                }
            }
        }).catch((err: any) => {
            console.log('Failed to delete squad share connections: ', err)
        }).finally(() => {
            this.deletePending = false
        })
    }

    mounted() {
        this.refreshConnections()
        this.refreshSquads()
    }
}

</script>