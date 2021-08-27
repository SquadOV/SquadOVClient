<template>
    <wow-generic-match-summary
        :match="challenge"
        :user-id="userId"
        :elapsed-time="elapsedTime"
        :mini="mini"
        :fill="fill"
        :disable-link="disableLink"
        :link-to-player-section="linkToPlayerSection"
        :access-token="accessToken"
        @go-to-character="$emit('go-to-character', arguments[0])"
    >
        <div class="text-subtitle-1 font-weight-bold">
            {{ challengeName }}
        </div>

        <div class="d-flex align-center text-subtitle-2">
            <v-img
                class="mr-2"
                src="assets/wow/Keystone.png"
                max-width="20px"
                max-height="20px"
                contain
            >
            </v-img>
            <span>+{{ challenge.keystoneLevel }}</span>
        </div>
    </wow-generic-match-summary>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import WowGenericMatchSummary from '@client/vue/utility/wow/WowGenericMatchSummary.vue'
import { Prop } from 'vue-property-decorator'
import { WowChallenge } from '@client/js/wow/matches'
import { millisecondsToTimeString } from '@client/js/time'

@Component({
    components: {
        WowGenericMatchSummary,
    }
})
export default class WowKeystoneSummary extends Vue {
    @Prop({required: true})
    challenge!: WowChallenge

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

    get challengeName(): string {
        return this.challenge.challengeName.replace(/"/g, '')
    }

    get elapsedTime(): string {
        return millisecondsToTimeString(this.challenge.timeMs)
    }
}

</script>