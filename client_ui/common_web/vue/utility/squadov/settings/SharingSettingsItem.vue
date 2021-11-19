<template>
    <loading-container :is-loading="settings === null">
        <template v-slot:default="{ loading }">
            <template v-if="!loading">
                <v-checkbox
                    v-if="$store.state.features.enableUserProfiles"
                    :input-value="isAutoSharingToProfile"
                    @change="toggleAutoShareToProfile"
                    label="Auto-Share VODs and Clips To Profile"
                    color="success"
                    hide-details
                >
                </v-checkbox>

                <template v-if="!!autoshareSettings">
                    <v-checkbox
                        v-model="autoshareSettings.shareOnJoin"
                        @change="changeAutoshareSettings"
                        label="Auto-Share Existing VODs and Clips to Squads on Join"
                        color="success"
                        hide-details
                        :disabled="autoshareOnJoinPending"
                    >
                        <template v-slot:append>
                            <v-progress-circular
                                v-if="autoshareOnJoinPending"
                                size="16"
                                indeterminate
                            >
                            </v-progress-circular>
                        </template>
                    </v-checkbox>
                </template>

                <v-tabs grow>
                    <v-tab>
                        Squads
                    </v-tab>

                    <v-tab-item>
                        <v-data-table
                            :headers="squadConnHeaders"
                            :items="squadConnItems"
                            :items-per-page="15"
                        >
                            <template v-slot:item.squad="{ item }">
                                <span class="font-weight-bold" v-if="!!item.squad">
                                    {{ item.squad.squadName }}
                                </span>
                            </template>

                            <template v-slot:item.share="{ item }">
                                <v-checkbox
                                    class="ma-0"
                                    :input-value="item.share"
                                    dense
                                    hide-details
                                    v-if="!!item.squad && !squadEditsPending[item.squad.id]"
                                    @change="editSquadSetting({...item.full, canShare: arguments[0]})"
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
                                    v-if="!!item.squad && !squadEditsPending[item.squad.id]"
                                    @change="editSquadSetting({...item.full, canClip: arguments[0]})"
                                >
                                </v-checkbox>

                                <v-progress-circular size="16" indeterminate v-else>
                                </v-progress-circular>
                            </template>

                            <template v-slot:item.games="{ item }">
                                <game-filter-ui
                                    :loading="!!item.squad && squadEditsPending[item.squad.id]"
                                    :value="item.games"
                                    @input="editSquadSetting({...item.full, games: arguments[0]})"
                                    style="max-width: 500px;"
                                    class="my-2"
                                >
                                </game-filter-ui>
                            </template>

                            <template v-slot:item.actions="{ item }">
                                <v-btn
                                    small
                                    icon
                                    color="error"
                                    :loading="deletePending"
                                    @click="deleteAutoSetting(item.full)"
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
                                @click="newSquadSetting"
                            >
                                Share
                            </v-btn>
                        </div>          
                    </v-tab-item>
                </v-tabs>
            </template>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import GameFilterUi from '@client/vue/utility/squadov/filters/GameFilterUi.vue'
import { AutoShareConnection, AutoShareSettings } from '@client/js/squadov/share'
import { apiClient, ApiData } from '@client/js/api'
import { Squad, SquadMembership } from '@client/js/squadov/squad'
import { allServerSideGames } from '@client/js/squadov/game'

@Component({
    components: {
        LoadingContainer,
        GameFilterUi
    }
})
export default class SharingSettingsItem extends Vue {
    settings: AutoShareConnection[] | null = null

    validSquads: Squad[] | null = null
    squadsToShare: Squad[] = []

    addPending: boolean = false
    deletePending: boolean = false
    squadEditsPending: { [squadId: number] : boolean | undefined } = {}
    isAutoSharingToProfile: boolean = false

    autoshareSettings: AutoShareSettings | null = null
    autoshareOnJoinPending: boolean = false

    changeAutoshareSettings() {
        if (!this.autoshareSettings) {
            return
        }

        this.autoshareOnJoinPending = true
        apiClient.editAutoShareSettings(this.autoshareSettings).catch((err: any) => {
            console.error('Failed to edit auto share settings: ', err)
        }).finally(() => {
            this.autoshareOnJoinPending = false
        })
    }

    toggleAutoShareToProfile(v: boolean) {
        if (!this.settings) {
            return
        }
        
        if (v) {
            apiClient.newAutoShareConnection({
                id: -1,
                sourceUserId: this.$store.state.currentUser.id,
                canShare: true,
                canClip: true,
                destUserId: null,
                destSquadId: null,
                games: allServerSideGames,
            }).then((resp: ApiData<AutoShareConnection>) => {
                if (!!this.settings) {
                    this.settings.push(resp.data)
                }
            }).catch((err: any) => {
                console.error('Failed to add auto share connection [profile]: ', err)
            })
        } else {
            let conn = this.settings.find((ele: AutoShareConnection) => ele.destUserId === null && ele.destSquadId === null)
            if (!conn) {
                return
            }

            this.deleteAutoSetting(conn)
        }
    }

    editSquadSetting(conn: AutoShareConnection) {
        if (conn.destSquadId === null) {
            return
        }

        Vue.set(this.squadEditsPending, conn.destSquadId, true)
        apiClient.editAutoShareConnection(conn.id, conn).then(() => {
            let editConn = this.groupedBySquad.get(conn.destSquadId!)!
            editConn.canShare = conn.canShare
            editConn.canClip = conn.canClip
            editConn.games = conn.games
        }).catch((err: any) => {
            console.error('Failed to edit auto share connection: ', err)
        }).finally(() => {
            Vue.set(this.squadEditsPending, conn.destSquadId!, false)
        })
    }

    newSquadSetting() {
        this.addPending = true
        Promise.all(this.squadsToShare.map((ele: Squad) => {
            return apiClient.newAutoShareConnection({
                id: -1,
                sourceUserId: this.$store.state.currentUser.id,
                canShare: true,
                canClip: true,
                destUserId: null,
                destSquadId: ele.id,
                games: allServerSideGames,
            })
        })).then((resp: ApiData<AutoShareConnection>[]) => {
            if (!!this.settings) {
                this.settings.push(...resp.map((ele: ApiData<AutoShareConnection>) => ele.data))
            }
        }).catch((err: any) => {
            console.error('Failed to add auto share connection: ', err)
        }).finally(() => {
            this.addPending = false
        })
    }

    deleteAutoSetting(conn: AutoShareConnection) {
        this.deletePending = true
        apiClient.deleteAutoShareConnection(conn.id).then(() => {
            if (!this.settings) {
                return
            }

            let idx = this.settings.findIndex((ele: AutoShareConnection) => ele.id == conn.id)
            if (idx === -1) {
                return
            }
            this.settings.splice(idx, 1)
        }).catch((err: any) => {
            console.error('Failed to delete auto share connection: ', err)
        }).finally(() => {
            this.deletePending = false
        })
    }

    get shareSquadItems(): any[] {
        if (!this.validSquads) {
            return []
        }
        return this.validSquads
            .filter((ele: Squad) => !this.groupedBySquad.has(ele.id))
            .map((ele: Squad) => {
                return {
                    text: `${ele.squadName}`,
                    value: ele,
                }
            })
    }

    get groupedBySquad(): Map<number, AutoShareConnection> {
        let ret: Map<number, AutoShareConnection> = new Map()
        if (!!this.settings) {
            for (let v of this.settings) {
                if (v.destSquadId !== null) {
                    ret.set(v.destSquadId, v)
                }
            }
        }
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

    refreshData() {
        apiClient.getAutoShareConnections().then((resp: ApiData<AutoShareConnection[]>) => {
            this.settings = resp.data
        }).catch((err: any) => {
            console.error('Failed to load auto share connections: ', err)
        })

        apiClient.getAutoShareSettings().then((resp: ApiData<AutoShareSettings>) => {
            this.autoshareSettings = resp.data
        }).catch((err: any) => {
            console.error('Failed to load auto share settings: ', err)
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
            console.error('Failed to get user squads [share connections editor]: ', err)
        })
    }

    @Watch('settings')
    refreshShareToProfile() {
        this.isAutoSharingToProfile = false
        if (!this.settings) {
            return
        }

        for (let s of this.settings) {
            if (s.destUserId === null && s.destSquadId === null) {
                this.isAutoSharingToProfile = true
                break
            }
        }
    }

    get squadConnHeaders(): any[] {
        return [
            {
                text: 'Squad',
                value: 'squad',
            },
            {
                text: 'Share',
                value: 'share',
            },
            {
                text: 'Clip',
                value: 'clip',
            },
            {
                text: 'Games',
                value: 'games',
            },
            {
                text: 'Actions',
                value: 'actions',
                sortable: false,
                filterable: false
            }
        ]
    }

    get squadConnItems(): any[] {
        let validSquads = Array.from(this.groupedBySquad.keys())
        
        return validSquads.map((id: number) => {
            let conns = this.groupedBySquad.get(id)!
            return {
                squad: this.squadMap.get(id)!,
                share: conns.canShare,
                clip: conns.canClip,
                games: conns.games,
                full: conns,
            }
        })
    }

    mounted() {
        this.refreshData()
        this.refreshSquads()
    }
}

</script>