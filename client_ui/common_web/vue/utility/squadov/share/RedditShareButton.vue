<template>
    <v-btn
        small
        icon
        @click="open"
    >
        <v-icon>
            mdi-reddit
        </v-icon>
    </v-btn>
</template>

<script lang="ts">

import Component, { mixins } from 'vue-class-component'
import GenericExternalShareButton from '@client/vue/utility/squadov/share/GenericExternalShareButton'
import { openUrlInBrowser } from '@client/js/external'

@Component
export default class RedditShareButton extends mixins(GenericExternalShareButton) {
    get redditUrl(): string {
        return `https://www.reddit.com/submit?url=${encodeURI(this.url)}&title=${encodeURI('Watch this!')}`
    }

    open() {
        openUrlInBrowser(this.redditUrl)
    }
}

</script>