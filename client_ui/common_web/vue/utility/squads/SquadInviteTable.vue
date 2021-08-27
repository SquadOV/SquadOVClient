<template>
    <div>
        <v-text-field
            outlined
            v-model="filterText"
            prepend-inner-icon="mdi-magnify"
            hide-details
            clearable
        >
        </v-text-field>

        <v-data-table
            :loading="value === null"
            loading-text="Loading..."
            :headers="headers"
            :items="items"
            :search="filterText"
        >
            <template v-slot:item.sent="{ item }">
                {{ standardFormatTime(item.sent) }}
            </template>
            <template v-slot:item.actions="{ item }">
                <v-btn icon color="error" v-if="isOwner" @click="revokeInvite(item)" :loading="item.revokePending">
                    <v-icon>
                        mdi-close-circle
                    </v-icon>
                </v-btn>
            </template>
        </v-data-table>

        <v-snackbar
            v-model="showHideError"
            :timeout="5000"
            color="error"
        >
            Failed to delete squad invite, please try again.
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import {
    SquadInvite,
} from '@client/js/squadov/squad'
import { standardFormatTime } from '@client/js/time'
import { apiClient, ApiData } from '@client/js/api' 

@Component
export default class SquadInviteTable extends Vue {
    standardFormatTime = standardFormatTime

    @Prop({required: true})
    value!: SquadInvite[]  | null

    @Prop({type: Boolean, default: false})
    isOwner!: boolean

    filterText:string = ''
    showHideError: boolean = false

    get currentUsername(): string {
        return this.$store.state.currentUser.username
    }

    get headers(): any[] {
        let ret : any[] = [
            {
                text: 'Recipient',
                value: 'recipient'
            },
            {
                text: 'Inviter',
                value: 'inviter'
            },
            {
                text: 'Sent',
                value: 'sent',
                sort: (a: Date, b: Date) => {
                    return a.getTime() - b.getTime()
                }
            },

        ]

        if (this.isOwner) {
            ret.push({
                text: 'Actions',
                value: 'actions',
                sortable: false,
                filterable: false
            })
        }

        return ret
    }

    get items(): any[] {
        if (!this.value) {
            return []
        }
        return this.value.map((ele: SquadInvite) => {
            return {
                recipient: !!ele.username ? ele.username : ele.email,
                inviter: ele.inviterUsername,
                sent: ele.inviteTime!,
                uuid: ele.inviteUuid,
                squadId: ele.squadId,
                revokePending: false,
            }
        })
    }

    revokeInvite(item : any) {
        if (!this.value) {
            return
        }

        Vue.set(item, 'revokePending', true)
        apiClient.revokeSquadInvite(item.squadId, item.uuid).then(() => {
            let newValues = this.value!.filter((ele: SquadInvite) => {
                return ele.inviteUuid != item.uuid
            })
            this.$emit('input', newValues)
            this.$emit('on-revoke')
        }).catch((err: any) => {
            this.showHideError = true
            console.error('Failed to revoke invite: ', err)
        }).finally(() => {
            Vue.set(item, 'revokePending', false)
        })
    }
}

</script>