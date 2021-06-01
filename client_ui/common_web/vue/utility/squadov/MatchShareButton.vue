<template>
    <div v-if="!!permissions && permissions.canShare">
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
                    Share this match!

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
                    <!-- Manual share settings -->
                    <div class="text-overline">
                        Share Settings
                    </div>
                    <v-divider class="mb-2"></v-divider>

                    <!-- Public share settings -->
                    <div class="text-overline mt-4">
                        Link Sharing
                    </div>
                    <v-divider class="mb-2"></v-divider>

                    <loading-container :is-loading="isSharedPublic === null">
                        <template v-slot:default="{ loading }">
                            <div v-if="!loading">
                                <template v-if="isSharedPublic">
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

                                    <div class="d-flex align-center">
                                        <v-btn
                                            text
                                            color="error"
                                            small
                                            v-if="!needConfirmDeleteLink"
                                            @click="needConfirmDeleteLink = true"
                                        >
                                            Delete Link
                                        </v-btn>
                                        
                                        <template v-else>
                                            <span class="text-overline">
                                                Are you sure you wish to delete this link?
                                            </span>

                                            <v-btn
                                                class="mx-1"
                                                icon
                                                color="success"
                                                small
                                                @click="deletePublicLink"
                                                :loading="deletingLink"
                                            >
                                                <v-icon>
                                                    mdi-check-circle
                                                </v-icon>
                                            </v-btn>

                                            <v-btn
                                                class="mx-1"
                                                icon
                                                color="error"
                                                small
                                                @click="needConfirmDeleteLink = false"
                                                :loading="deletingLink"
                                            >
                                                <v-icon>
                                                    mdi-close-circle
                                                </v-icon>
                                            </v-btn>
                                        </template>
                                    </div>
                                </template>

                                <template v-else>
                                    <div class="d-flex align-center">
                                        <v-btn
                                            block
                                            color="success"
                                            @click="generatePublicLink"
                                            :loading="creatingLink"
                                        >
                                            Create Link
                                        </v-btn>
                                    </div>

                                    <div class="text-caption">
                                        Warning: Everyone with this link will be able to view this match and the VODs you <b>currently</b> have share access to until you delete this link!
                                    </div>
                                </template>
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
            Oops! Something went wrong, please try again later (or submit a bug report)!
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { SquadOvGames } from '@client/js/squadov/game'
import { StatPermission } from '@client/js/stats/statPrimitives'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { LinkShareData, MatchVideoSharePermissions } from '@client/js/squadov/share'

@Component({
    components: {
        LoadingContainer
    }
})
export default class MatchShareButton extends Vue {
    @Prop({required: true})
    matchUuid!: string

    @Prop({required: true})
    game!: SquadOvGames

    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    permissions!: MatchVideoSharePermissions | null

    @Prop()
    graphqlStats!: StatPermission[]

    showHideError: boolean = false
    showHideShare: boolean = false

    isSharedPublic: boolean | null = null
    shareUrl: string | null = null
    creatingLink: boolean = false
    needConfirmDeleteLink: boolean = false
    deletingLink: boolean = false

    messages: string[] = []

    $refs!: {
        urlInput: any
    }

    handlePublicLinkResponse(resp: LinkShareData) {
        this.isSharedPublic = resp.isLinkShared
        this.shareUrl = resp.shareUrl
    }

    generatePublicLink() {
        this.creatingLink = true
        apiClient.createMatchShareUrl(this.matchUuid, this.$route.fullPath, this.game, this.graphqlStats).then((resp: ApiData<LinkShareData>) => {
            this.handlePublicLinkResponse(resp.data)
        }).catch((err: any) => {
            this.showHideError = true
            console.log('Failed to create share URL: ', err)
        }).finally(() => {
            this.creatingLink = false
        })
    }

    deletePublicLink() {
        this.deletingLink = true
        apiClient.deleteMatchShareUrl(this.matchUuid).then(() => {
            this.isSharedPublic = false
            this.shareUrl = null
        }).catch((err: any) => {
            this.showHideError = true
            console.log('Failed to delete share URL: ', err)
        }).finally(() => {
            this.deletingLink = false
        })
    }

    @Watch('$route.fullPath')
    @Watch('matchUuid')
    refreshLinkShareData() {
        this.isSharedPublic = null
        this.shareUrl = null
        apiClient.getMatchShareUrl(this.matchUuid).then((resp: ApiData<LinkShareData>) => {
            this.handlePublicLinkResponse(resp.data)
        }).catch((err: any) => {
            this.showHideError = true
            console.log('Failed to get share URL: ', err)
        })
    }

    onShow() {
        this.refreshLinkShareData()
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