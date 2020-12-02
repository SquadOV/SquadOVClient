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
                <v-btn icon color="error" v-if="isOwner">
                    <v-icon>
                        mdi-close-circle
                    </v-icon>
                </v-btn>
            </template>
        </v-data-table>
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

@Component
export default class SquadInviteTable extends Vue {
    standardFormatTime = standardFormatTime

    @Prop({required: true})
    value!: SquadInvite[]  | null

    @Prop({type: Boolean, default: false})
    isOwner!: boolean

    filterText:string = ''

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
                recipient: ele.username,
                inviter: ele.inviterUsername,
                sent: ele.inviteTime!,
            }
        })
    }
}

</script>