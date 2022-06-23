<template>
    <div class="d-flex align-center full-width">
        <div :class="shrink ? 'flex-grow-0' : 'flex-grow-1'">
            <slot v-bind:disabled="disabled"></slot>
        </div>

        <div class="ml-2" v-if="disabled || forceShow">
            <v-tooltip bottom max-width="400px" :open-delay="250">
                <template v-slot:activator="{ on, attrs }">
                    <sub-icon
                        :tier="tier"
                        v-on="on"
                        v-bind="attrs"
                    >
                    </sub-icon>
                </template>

                <span v-if="!forceShow">This feature requires a {{ tier }} tier subscription to SquadOV.</span> Check out SquadOV Pro today!
            </v-tooltip>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { EPricingTier } from '@client/js/squadov/pricing'
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

    @Prop({type: Boolean, default: false})
    shrink!: boolean

    @Prop({type: Boolean, default: false})
    forceShow!: boolean

    get disabled(): boolean {
        return !this.$store.getters.isUserInTier(this.tier)
    }
}

</script>