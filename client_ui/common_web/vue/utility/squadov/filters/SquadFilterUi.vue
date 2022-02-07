<template>
    <v-autocomplete
        label="Squads"
        :value="value"
        @input="changeValue"
        :items="items"
        deletable-chips
        chips
        multiple
        clearable
        outlined
        hide-details
        :loading="!availableSquads"
        dense
    >
    </v-autocomplete>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { SquadMembership } from '@client/js/squadov/squad'
import { apiClient, ApiData } from '@client/js/api'

@Component
export default class SquadFilterUi extends Vue {
    @Prop({type: Array})
    value!: number[] | undefined

    availableSquads: SquadMembership[] | null = null

    changeValue(v: number[]) {
        if (v.length === 0) {
            this.$emit('input', undefined)
        } else {
            this.$emit('input', v)
        }
    }

    get items(): any[] {
        if (!this.availableSquads) {
            return []
        }
        return this.availableSquads.map((s: SquadMembership) => ({
           text: `${s.squad.squadName}`,
           value: s.squad.id, 
        }))
    }

    refreshSquads() {
        apiClient.getUserSquads(this.$store.state.currentUser.id).then((resp: ApiData<SquadMembership[]>) => {
            this.availableSquads = resp.data
        }).catch((err: any) => {
            console.error('Failed to obtain user squads: ', err)
        })
    }

    mounted() {
        this.refreshSquads()
    }
}

</script>