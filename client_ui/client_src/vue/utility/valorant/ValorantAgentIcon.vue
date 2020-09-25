<template>
    <v-img
        :src="agentFname"
        :max-width="widthHeight"
        :max-height="widthHeight"
        contain
    >
    </v-img>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { getValorantContent } from '@client/js/valorant/valorant_content'

@Component
export default class ValorantAgentIcon extends Vue {
    @Prop({required: true})
    agent! : string

    @Prop({required: true})
    patch! : string

    @Prop({type: Number, default: 50})
    widthHeight!: number

    get agentName() : string {
        let cnt = getValorantContent(this.patch)
        return cnt.agentIdToName(this.agent)
    }

    get agentFname() : string {
        return `assets/valorant/agents/${this.agentName}.png`
    }
}

</script>

<style scoped>

</style>