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
import CommonComponent from '@client/vue/CommonComponent'
import GenericExternalShareButton from '@client/vue/utility/squadov/share/GenericExternalShareButton'
import { openUrlInBrowser } from '@client/js/external'

@Component
export default class RedditShareButton extends mixins(CommonComponent, GenericExternalShareButton) {
    get redditUrl(): string {
        return `https://www.reddit.com/submit?url=${encodeURI(this.url)}&title=${encodeURI('Watch this!')}`
    }

    open() {
        this.sendAnalyticsEvent(this.AnalyticsCategory.Share, this.AnalyticsAction.ShareSocialMedia, 'Reddit', 0)
        openUrlInBrowser(this.redditUrl)
    }
}

</script>