<template>
    <loading-container :is-loading="true">
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { ShareAccessTokenResponse } from '@client/js/squadov/share'
import { apiClient, ApiData } from '@client/js/api'
import * as pi from '@client/js/pages'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'

@Component({
    components: {
        LoadingContainer
    }
})
export default class ShareRedirect extends Vue {
    @Prop({required: true})
    accessTokenId!: string

    mounted() {
        // Get final redirect URL by processing the access token on the server side.
        // Processing the access token will also grant us a temporary key that'll be able to emulate
        // the user who shared the URL on the desired endpoints.
        apiClient.exchangeShareAccessToken(this.accessTokenId).then((resp: ApiData<ShareAccessTokenResponse>) => {
            let url = new URL(resp.data.fullPath, 'http://localhost')
            let params = new URLSearchParams(url.search)
            params.append('share', resp.data.key)
            params.append('uid', resp.data.uid.toString())
            url.search = params.toString()
            this.$router.replace(url.pathname + '?' + params.toString())
        }).catch((err: any) => {
            console.error('Failed to exchange access token: ', err)
            this.$router.replace({
                name: pi.DashboardPageId
            })
        })
    }
}

</script>