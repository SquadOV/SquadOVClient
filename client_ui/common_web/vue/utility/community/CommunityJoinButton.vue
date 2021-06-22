<template>
    <v-btn
        color="primary"
        small
        @click="join"
    >
        Join
    </v-btn>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { SquadOvCommunity } from '@client/js/squadov/community'

@Component
export default class CommunityJoinButton extends Vue {
    @Prop({required: true})
    community!: SquadOvCommunity

    join() {

    }
}

</script>