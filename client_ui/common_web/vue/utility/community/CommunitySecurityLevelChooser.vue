<template>
    <v-select
        :value="value"
        @input="$emit('input', arguments[0])"
        hide-details
        label="Visibility"
        :items="items"
    >
    </v-select>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { CommunitySecurityLevel } from '@client/js/squadov/community'

@Component
export default class CommunitySecurityLevelChooser extends Vue {
    @Prop({required: true})
    value!: CommunitySecurityLevel

    get items(): any[] {
        return [
            {
                text: 'Public',
                value: CommunitySecurityLevel.Public
            },
            {
                text: 'Private',
                value: CommunitySecurityLevel.Private
            },
            {
                text: 'Invite-Only',
                value: CommunitySecurityLevel.Unlisted
            },
        ]
    }
}

</script>