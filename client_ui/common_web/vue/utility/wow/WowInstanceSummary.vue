<template>
    <wow-generic-match-summary
        :match="instance"
        :user-id="userId"
        elapsed-time=""
        :mini="mini"
        :fill="fill"
        :disable-link="disableLink"
        :link-to-player-section="linkToPlayerSection"
        :access-token="accessToken"
        @go-to-character="$emit('go-to-character', arguments[0])"
    >
        <div class="text-subtitle-1 font-weight-bold">
            {{ wowInstanceTypeToName(instance.instanceType) }}
        </div>
    </wow-generic-match-summary>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import WowGenericMatchSummary from '@client/vue/utility/wow/WowGenericMatchSummary.vue'
import { Prop } from 'vue-property-decorator'
import { WowInstance, wowInstanceTypeToName } from '@client/js/wow/matches'

@Component({
    components: {
        WowGenericMatchSummary,
    }
})
export default class WowInstanceSummary extends Vue {
    wowInstanceTypeToName = wowInstanceTypeToName

    @Prop({required: true})
    instance!: WowInstance

    @Prop({type: Number, required: true})
    userId!: number

    @Prop({type: Boolean, default: false})
    mini!: boolean

    @Prop({type: Boolean, default: false})
    fill!: boolean

    @Prop({type: Boolean, default: false})
    disableLink!: boolean

    @Prop({type: Boolean, default: false})
    linkToPlayerSection!: boolean

    @Prop()
    accessToken!: string | undefined
}

</script>