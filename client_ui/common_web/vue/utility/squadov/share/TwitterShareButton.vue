<template>
    <v-btn
        small
        icon
        @click="open"
    >
        <v-icon>
            mdi-twitter
        </v-icon>
    </v-btn>
</template>

<script lang="ts">

import Component, { mixins } from 'vue-class-component'
import GenericExternalShareButton from '@client/vue/utility/squadov/share/GenericExternalShareButton'
import { openUrlInBrowser } from '@client/js/external'

@Component
export default class TwitterShareButton extends mixins(GenericExternalShareButton) {
    get twitterUrl(): string {
        return `https://www.twitter.com/intent/tweet?text=${encodeURI('Check this out! ' + this.url + ' @SquadOV')}`
    }

    open() {
        openUrlInBrowser(this.twitterUrl)
    }
}

</script>