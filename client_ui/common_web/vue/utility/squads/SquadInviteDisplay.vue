<template>
    <loading-container :is-loading="!relevantSquad">
        <template v-slot:default="{ loading }">
            <v-card class="full-width" :elevation="2" v-if="!loading">
                <div class="d-flex">
                    <div>
                        <v-card-title>
                            {{ relevantSquad.squadName }}
                        </v-card-title>

                        <v-card-subtitle>
                            {{ relevantSquad.squadGroup }} ({{ invite.inviterUsername }})
                        </v-card-subtitle>
                    </div>

                    <v-spacer></v-spacer>

                    <div class="d-flex align-center mx-4">
                        <div>
                            <v-btn icon color="success" @click="acceptInvite" :loading="pending">
                                <v-icon>
                                    mdi-check-circle
                                </v-icon>
                            </v-btn>
                        </div>

                        <div class="ml-2">
                            <v-btn icon color="error" @click="rejectInvite" :loading="pending">
                                <v-icon>
                                    mdi-close-circle
                                </v-icon>
                            </v-btn>
                        </div>
                    </div>
                </div>
            </v-card>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { Prop, Watch } from 'vue-property-decorator'
import { Squad, SquadInvite } from '@client/js/squadov/squad'
import { apiClient, ApiData } from '@client/js/api' 

@Component({
    components: {
        LoadingContainer
    }
})
export default class SquadInviteDisplay extends Vue {
    @Prop({required: true})
    invite!: SquadInvite

    relevantSquad: Squad | null = null
    pending: boolean = false

    @Watch('invite')
    refreshData() {
        apiClient.getSquad(this.invite.squadId).then((resp: ApiData<Squad>) => {
            this.relevantSquad = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain squad for invite: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }

    acceptInvite() {
        this.pending = true
        apiClient.acceptSquadInvite(this.invite.squadId, this.invite.inviteUuid).then(() => {
            this.$emit('clear-invite')
        }).catch((err: any) => {
            console.log('Failed to accept invite: ', err)
        }).finally(() => {
            this.pending = false
        })
    }

    rejectInvite() {
        this.pending = true
        apiClient.rejectSquadInvite(this.invite.squadId, this.invite.inviteUuid).then(() => {
            this.$emit('clear-invite')
        }).catch((err: any) => {
            console.log('Failed to reject invite: ', err)
        }).finally(() => {
            this.pending = false
        })
    }
}

</script>