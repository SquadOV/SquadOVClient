<template>
    <div class="ma-4">
        <div class="d-flex">
            <div class="text-h5 font-weight-bold">
                My Squads
            </div>

            <v-spacer></v-spacer>

            <div>
                <v-dialog
                    v-model="showHideCreateSquad"
                    persistent
                    width="40%"
                >
                    <template v-slot:activator="{on, attrs}">
                        <v-btn color="primary" v-on="on" v-bind="attrs">
                            Create Squad
                        </v-btn>
                    </template>

                    <squad-creation-card
                        @on-new-squad="onNewSquad"
                        @cancel-new-squad="showHideCreateSquad = false"
                    >
                    </squad-creation-card>
                </v-dialog>
            </div>
        </div>

        <v-divider class="my-4"></v-divider>

        <v-tabs>
            <v-tab>
                Squads
            </v-tab>

            <v-tab-item>
                <loading-container :is-loading="!mySquads">
                    <template v-slot:default="{ loading }">
                        <v-container fluid  v-if="!loading">
                            <v-row>
                                <v-col cols="3" v-for="(membership, idx) in mySquads" :key="`col-membership-${idx}`">
                                    <squad-membership-summary-display
                                        :key="`membership-${idx}`"
                                        :membership="membership"
                                    >
                                    </squad-membership-summary-display>
                                </v-col>
                            </v-row>
                        </v-container>
                    </template>
                </loading-container>
            </v-tab-item>

            <v-tab>
                <v-badge
                    inline
                    :value="totalInvites > 0"
                    :content="`${totalInvites}`"
                    color="error"
                >
                    Invites
                </v-badge>
            </v-tab>

            <v-tab-item>
                <loading-container :is-loading="!myInvites">
                    <template v-slot:default="{ loading }">
                        <v-container fluid  v-if="!loading">
                            <v-row>
                                <v-col cols="3" v-for="(invite, idx) in myInvites" :key="`col-invite-${idx}`">
                                    <squad-invite-display
                                        :key="`invite-${invite.inviteUuid}`"
                                        :invite="invite"
                                        @clear-invite="removeInvite(invite)"
                                    >
                                    </squad-invite-display>
                                </v-col>
                            </v-row>
                        </v-container>
                    </template>
                </loading-container>
            </v-tab-item>
        </v-tabs>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api' 
import { SquadMembership, SquadInvite } from '@client/js/squadov/squad'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import SquadMembershipSummaryDisplay from '@client/vue/utility/squads/SquadMembershipSummaryDisplay.vue'
import SquadInviteDisplay from '@client/vue/utility/squads/SquadInviteDisplay.vue'
import SquadCreationCard from '@client/vue/utility/squads/SquadCreationCard.vue'

@Component({
    components: {
        LoadingContainer,
        SquadMembershipSummaryDisplay,
        SquadInviteDisplay,
        SquadCreationCard
    }
})
export default class UserSquads extends Vue {
    @Prop({required: true})
    userId!: number

    showHideCreateSquad: boolean = false

    mySquads: SquadMembership[] | null = null
    myInvites: SquadInvite[] | null = null

    get totalInvites(): number {
        if (!this.myInvites) {
            return 0
        }
        return this.myInvites.length
    }

    @Watch('userId')
    refreshData() {
        this.mySquads = null
        this.myInvites = null

        apiClient.getUserSquads(this.userId).then((resp: ApiData<SquadMembership[]>) => {
            this.mySquads = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain user squads: ', err)
        })

        apiClient.getUserSquadInvites(this.userId).then((resp: ApiData<SquadInvite[]>) => {
            this.myInvites = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain user squad invites: ', err)
        })
    }

    onNewSquad() {
        this.showHideCreateSquad = false
        this.refreshData()
    }
    
    removeInvite(inv: SquadInvite) {
        if (!this.myInvites) {
            return
        }
        let idx = this.myInvites.findIndex((ele: SquadInvite) => {
            return ele.inviteUuid = inv.inviteUuid
        })

        if (idx == -1) {
            return
        }

        this.myInvites.splice(idx, 1)
    }

    mounted() {
        this.refreshData()
    }
}

</script>

<style scoped>

>>>.v-tabs-items {
    background-color: inherit !important;
}

</style>