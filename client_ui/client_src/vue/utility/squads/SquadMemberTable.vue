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
            <template v-slot:item.actions="{ item }">
                <v-btn icon color="error" v-if="isOwner && item.role != 'Owner'">
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
    SquadMembership,
    SquadRole
} from '@client/js/squadov/squad'

@Component
export default class SquadMemberTable extends Vue {
    @Prop({required: true})
    value!: SquadMembership[]  | null

    @Prop({type: Boolean, default: false})
    isOwner!: boolean

    filterText:string = ''
    
    get headers(): any[] {
        let ret : any[] = [
            {
                text: 'Username',
                value: 'username'
            },
            {
                text: 'Role',
                value: 'role'
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
        return this.value.map((ele : SquadMembership) => {
            return {
                username: ele.username,
                role: SquadRole[ele.role],
            }
        })
    }
}

</script>