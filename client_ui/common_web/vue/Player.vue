<template>
    <video-player
        class="full-parent-height full-width"
        v-if="!!vod"
        fill
        disable-theater
        disable-popout
        :vod="vod"
    >
    </video-player>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { ShareAccessTokenResponse } from '@client/js/squadov/share'
import { apiClient, ApiData } from '@client/js/api'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'
import { VodAssociation } from '@client/js/squadov/vod'

@Component({
    components: {
        VideoPlayer
    }
})
export default class Player extends Vue {
    @Prop({required: true})
    vodUuid!: string

    @Prop()
    share!: string | null
    hasAccess: boolean = false
    vod: VodAssociation | null = null

    requestVod() {
        apiClient.findVodFromVideoUuid(this.vodUuid).then((resp: ApiData<VodAssociation>) => {
            this.vod = resp.data
        }).catch((err: any) => {
            console.error('Failed to get VOD: ', err)
        })
    }

    @Watch('share')
    requestAccess() {
        if (!!this.share) {
            this.hasAccess = false
            apiClient.exchangeShareAccessToken(this.share).then((resp: ApiData<ShareAccessTokenResponse>) => {
                apiClient.setTempSessionId(resp.data.key, resp.data.uid.toString())
                this.hasAccess = true
                this.requestVod()
            }).catch((err: any) => {
                console.error('Failed to exchange access token: ', err)
            })
        } else if (!!this.$store.state.currentUser) {
            this.requestVod()
        }
    }

    mounted() {
        this.requestAccess()
    }
}

</script>