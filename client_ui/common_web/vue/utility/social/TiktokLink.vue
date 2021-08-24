<template>
    <generic-social-link
        :url="fullLink"
        :label="`@${input}`"
        :icon="$root.generateAssetUri('assets/profile/social/tiktok.png')"
    >
    </generic-social-link>
</template>

<script lang="ts">

import Vue from 'vue'
import GenericSocialLink from '@client/vue/utility/social/GenericSocialLink.vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'

@Component({
    components: {
        GenericSocialLink,
    }
})
export default class TwitterLink extends Vue {
    @Prop({required: true})
    input!: string

    get fullLink(): string {
        return `https://www.tiktok.com/@${this.input}`
    }
}

</script>