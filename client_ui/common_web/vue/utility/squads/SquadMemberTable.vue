<template>
    <div>
        <v-dialog
            v-model="showHideDelete"
            persistent
            width="40%"
        >
            <v-card v-if="!!userToDelete">
                <v-card-title>
                    Kick {{ userToDelete.username }}?
                </v-card-title>
                <v-divider></v-divider>

                <v-card-text class="mt-4">
                    Are you sure you wish to remove {{ userToDelete.username }} from the squad?
                </v-card-text>

                <v-card-actions>
                    <v-btn @click="cancelKick">
                        Cancel
                    </v-btn>

                    <v-spacer></v-spacer>

                    <v-btn color="error" :loading="kickPending" @click="doKick">
                        Kick
                    </v-btn>
                </v-card-actions>
            </v-card>
        </v-dialog>

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
                <v-btn icon color="error" v-if="isOwner && item.role != 'Owner'" @click="stageKick(item.value)">
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
            Failed to remove user from squad, please try again.
        </v-snackbar>
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
import { apiClient, ApiData } from '@client/js/api' 

@Component
export default class SquadMemberTable extends Vue {
    @Prop({required: true})
    value!: SquadMembership[]  | null

    @Prop({type: Boolean, default: false})
    isOwner!: boolean

    filterText:string = ''
    showHideError: boolean = false

    showHideDelete: boolean = false
    userToDelete: SquadMembership | null = null
    kickPending: boolean = false
    
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
        return [...this.value]
            .sort((a: SquadMembership, b: SquadMembership) => {
                if (a.username < b.username) {
                    return -1
                } else if (a.username > b.username) {
                    return 1
                } else {
                    return 0
                }
            })
            .map((ele : SquadMembership) => {
                return {
                    username: ele.username,
                    role: SquadRole[ele.role],
                    value: ele,
                }
            })
    }

    cancelKick() {
        this.userToDelete = null
        this.showHideDelete = false
    }

    doKick() {
        if (!this.userToDelete) {
            return
        }

        this.kickPending = true
        apiClient.kickSquadMember(this.userToDelete.squad.id, this.userToDelete.userId).then(() => {
            let newValues = this.value!.filter((ele: SquadMembership) => {
                return ele.userId != this.userToDelete!.userId
            })
            this.$emit('input', newValues)
            this.$emit('on-kick')
            this.cancelKick()
        }).catch((err: any) => {
            this.showHideError = true
            console.error('Failed to kick player: ', err)
        }).finally(() => {
            this.kickPending = false
        })
    }

    stageKick(user: SquadMembership) {
        this.userToDelete = user
        this.showHideDelete = true
    }
}

</script>