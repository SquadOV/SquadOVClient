<template>
    <div>
        <v-btn
            icon
            color="primary"
            @click="onShow"
        >
            <v-icon>
                mdi-share-variant
            </v-icon>
        </v-btn>
        <v-dialog
            v-model="showHideShare"
            max-width="40%"
        >
            <v-card>
                <v-card-title>
                    Share your match!

                    <v-spacer></v-spacer>

                    <v-btn
                        icon
                        small
                        @click="showHideShare = false"
                    >
                        <v-icon>
                            mdi-close
                        </v-icon>
                    </v-btn>
                </v-card-title>
                <v-divider></v-divider>

                <div class="pa-4">
                    <loading-container :is-loading="!shareUrl">
                        <template v-slot:default="{ loading }">
                            <div v-if="!loading">
                                <div class="d-flex align-center">
                                    <v-text-field
                                        :value="shareUrl"
                                        :success-messages="messages"
                                        single-line
                                        outlined
                                        dense
                                        readonly
                                        ref="urlInput"
                                    >
                                        <template v-slot:append-outer>
                                            <v-btn
                                                icon
                                                color="success"
                                                @click="doCopy"
                                            >
                                                <v-icon>
                                                    mdi-content-copy
                                                </v-icon>
                                            </v-btn>
                                        </template>
                                    </v-text-field>
                                </div>

                                <div class="text-caption">
                                    Warning: Everyone with this link will be able to view this clip!
                                </div>
                            </div>
                        </template>
                    </loading-container>
                </div>
            </v-card>
        </v-dialog>

        <v-snackbar
            v-model="showHideError"
            :timeout="5000"
            color="error"
        >
            Oops! Something went wrong when generating the link, try again later!
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { SquadOvGames } from '@client/js/squadov/game'
import { StatPermission } from '@client/js/stats/statLibrary'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'

@Component({
    components: {
        LoadingContainer
    }
})
export default class ClipShareButton extends Vue {
    @Prop({required: true})
    clipUuid!: string

    showHideError: boolean = false
    showHideShare: boolean = false
    shareUrl: string | null = null
    messages: string[] = []

    $refs!: {
        urlInput: any
    }

    @Watch('$route.fullPath')
    @Watch('clipUuid')
    regenerateShareUrl() {
        this.shareUrl = null
        apiClient.getClipShareUrl(this.clipUuid, this.$route.fullPath).then((resp: ApiData<string>) => {
            this.shareUrl = resp.data
        }).catch((err: any) => {
            this.showHideError = true
            console.log('Failed to get share URL: ', err)
        })
    }

    onShow() {
        this.regenerateShareUrl()
        this.showHideShare = true
    }

    doCopy() {
        let inputEle = this.$refs.urlInput.$el.querySelector('input')
        inputEle.select()
        document.execCommand('copy')
        this.messages = ['Copied URL to clipboard!']
        setTimeout(() => {
            this.messages = []
        }, 5000)
    }
}

</script>