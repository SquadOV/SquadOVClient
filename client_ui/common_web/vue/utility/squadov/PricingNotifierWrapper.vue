<template>
    <div class="d-flex align-center full-width">
        <div :class="shrink ? 'flex-grow-0' : 'flex-grow-1'">
            <slot v-bind:disabled="disabled"></slot>
        </div>

        <div class="ml-2" v-if="disabled || forceShow">
            <v-tooltip bottom max-width="400px" :open-delay="250">
                <template v-slot:activator="{ on, attrs }">
                    <div
                        v-on="on"
                        v-bind="attrs"
                    >
                        <sub-icon
                            :tier="tier"
                        >
                        </sub-icon>
                    </div>
                </template>

                <span v-if="!forceShow">{{ tooltip }}</span> Check out SquadOV Pro {{ tier }} today!
            </v-tooltip>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { EPricingTier, EProFeature } from '@client/js/squadov/pricing'
import { PricingPageId } from '@client/js/pages'
import SubIcon from '@client/vue/utility/squadov/SubIcon.vue'

@Component({
    components: {
        SubIcon
    }
})
export default class PricingNotifierWrapper extends Vue {
    @Prop({required: true})
    tier!: EPricingTier

    @Prop({required: true})
    feature!: EProFeature

    @Prop({type: Boolean, default: false})
    shrink!: boolean

    @Prop({type: Boolean, default: false})
    forceShow!: boolean

    get disabled(): boolean {
        return !this.$store.getters.isUserInTier(this.tier)
    }

    get tooltip(): string {
        switch (this.feature) {
            case EProFeature.Support:
                return 'SquadOV Pro users get higher priority support.'
            case EProFeature.SquadSize:
                return 'SquadOV Pro users can have more people join their squads.'
            case EProFeature.Watermark:
                return 'SquadOV Pro users can remove the SquadOV watermark.'
            case EProFeature.Resolution:
                return 'SquadOV Pro users can record at higher resolutions.'
            case EProFeature.Fps:
                return 'SquadOV Pro users can record videos with higher FPS.'
            case EProFeature.Bitrate:
                return 'SquadOV Pro users can record videos with higher quality (bitrate).'
            case EProFeature.Codec:
                return 'The video codec feature is currently in early access.'
            case EProFeature.VodRetention:
                return 'SquadOV Pro users will have their videos stored on the cloud for longer.'
            case EProFeature.ClipLength:
                return 'SquadOV Pro users can create clips that are longer.'
            case EProFeature.SeparateAudioTracks:
                return 'SquadOV Pro users can get each audio device/process into a separate tracks to make it easier to create content.'
        }
    }
}

</script>