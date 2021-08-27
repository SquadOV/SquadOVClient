<template>
    <v-select
        label="Users"
        :value="value"
        @input="changeValue"
        :items="items"
        deletable-chips
        chips
        multiple
        clearable
        outlined
        hide-details
        :loading="!availableUsers"
        dense
    >
    </v-select>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { SquadOVUserHandle } from '@client/js/squadov/user'
import { apiClient, ApiData } from '@client/js/api'

@Component
export default class UserFilterUi extends Vue {
    @Prop({type: Array})
    value!: number[] | undefined

    @Prop({type: Array})
    squads!: number[] | undefined

    availableUsers: SquadOVUserHandle[] | null = null

    changeValue(v: number[]) {
        if (v.length === 0) {
            this.$emit('input', undefined)
        } else {
            this.$emit('input', v)
        }
    }

    get items(): any[] {
        if (!this.availableUsers) {
            return []
        }
        return this.availableUsers.map((s: SquadOVUserHandle) => ({
           text: s.username,
           value: s.id,
        }))
    }

    @Watch('squads')
    refreshUsers() {
        apiClient.getMySquadMates(this.squads).then((resp: ApiData<SquadOVUserHandle[]>) => {
            this.availableUsers = resp.data
        }).catch((err: any) => {
            console.error('Failed to get squad mates: ', err)
        })
    }

    mounted() {
        this.refreshUsers()
    }
}

</script>