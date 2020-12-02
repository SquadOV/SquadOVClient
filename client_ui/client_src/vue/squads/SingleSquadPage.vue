<template>
    <loading-container :is-loading="!localMembership">
        <template v-slot:default="{ loading }">
            <v-container fluid v-if="!loading">
                <div class="d-flex">
                    <div>
                        <span class="text-h5 font-weight-bold">{{ localMembership.squad.squadName }}</span>
                        <span class="text-subtitle-2 squad-group-text">{{ localMembership.squad.squadGroup }}</span>
                    </div>
                    <v-spacer></v-spacer>
                </div>
                <v-divider class="my-4"></v-divider>

                <v-tabs>
                    <v-tab>
                        Members ({{ localMembership.squad.memberCount }})
                    </v-tab>

                    <v-tab-item>
                        <squad-member-table
                            class="ma-4"
                            v-model="squadMembers"
                            :is-owner="isOwner"
                        >
                        </squad-member-table>
                    </v-tab-item>

                    <v-tab>
                        Invites ({{ localMembership.squad.pendingInviteCount }})
                    </v-tab>

                    <v-tab-item>
                        <squad-invite-table
                            class="ma-4"
                            v-model="squadInvites"
                            :is-owner="isOwner"
                        >
                        </squad-invite-table>
                    </v-tab-item>
                </v-tabs>
            </v-container>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api' 
import {
    SquadMembership,
    SquadInvite,
    SquadRole
} from '@client/js/squadov/squad'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import SquadMemberTable from '@client/vue/utility/squads/SquadMemberTable.vue'
import SquadInviteTable from '@client/vue/utility/squads/SquadInviteTable.vue'

@Component({
    components: {
        LoadingContainer,
        SquadMemberTable,
        SquadInviteTable
    }
})
export default class SingleSquadPage extends Vue {
    @Prop({required: true})
    squadId!: number

    localMembership: SquadMembership | null = null
    squadMembers: SquadMembership[] | null = null
    squadInvites: SquadInvite[] | null = null

    get isOwner(): boolean {
        return this.localMembership!.role == SquadRole.Owner
    }

    @Watch('squadId')
    refreshData() {
        apiClient.getUserSquadMembership(this.squadId, this.$store.state.currentUser.id).then((resp: ApiData<SquadMembership>) => {
            this.localMembership = resp.data
        }).catch((err: any) => {
            console.log('Failed to get local squad membership: ', err)
        })

        apiClient.getSquadUsers(this.squadId).then((resp: ApiData<SquadMembership[]>) => {
            this.squadMembers = resp.data
        }).catch((err: any) => {
            console.log('Failed to get all squad memberships: ', err)
        })

        apiClient.getSquadInvites(this.squadId).then((resp: ApiData<SquadInvite[]>) => {
            this.squadInvites = resp.data
        }).catch((err: any) => {
            console.log('Failed to get all squad invites: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>

<style scoped>

.squad-group-text {
    color: rgba(255, 255, 255, 0.7);
}

</style>