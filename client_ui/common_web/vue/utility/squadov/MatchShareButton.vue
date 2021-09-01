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
                    {{ !!matchUuid ? 'Share this match!' : 'Share this clip!' }}

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
                    <!-- Public share settings -->
                    <div class="text-overline">
                        Link Sharing
                    </div>
                    <v-divider class="mb-2"></v-divider>
                    
                    <v-checkbox
                        v-if="$store.state.features.enableUserProfiles && isSharedProfile !== null && isSharedProfile.canShare"
                        :input-value="isSharedProfile.isShared"
                        @change="toggleSharedToProfile"
                        label="Shared to Profile"
                        color="success"
                        class="mb-2"
                        hide-details
                    >
                    </v-checkbox>

                    <loading-container :is-loading="isSharedPublic === null">
                        <template v-slot:default="{ loading }">
                            <div v-if="!loading">
                                <template v-if="isSharedPublic">
                                    <div class="d-flex align-center">
                                        <v-text-field
                                            :value="finalShareUrl"
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

                                    <v-checkbox
                                        v-model="shareTimestamp"
                                        label="Share Timestamp"
                                        class="my-2"
                                        hide-details
                                    >
                                    </v-checkbox>

                                    <div class="d-flex align-center">
                                        <template v-if="!needConfirmDeleteLink">
                                            <v-btn
                                                text
                                                color="error"
                                                small
                                                @click="needConfirmDeleteLink = true"
                                            >
                                                Delete Link
                                            </v-btn>
                                            <v-spacer></v-spacer>
                                            <facebook-share-button class="mx-1" :url="shareUrl"></facebook-share-button>
                                            <reddit-share-button class="mx-1" :url="shareUrl"></reddit-share-button>
                                            <twitter-share-button class="mx-1" :url="shareUrl"></twitter-share-button> 
                                        </template>
                                        
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

                                    <div class="text-caption font-weight-bold mt-1">
                                        {{ warningText }}
                                    </div>
                                </template>
                            </div>
                        </template>
                    </loading-container>

                    <!-- Manual share settings -->
                    <div class="text-overline mt-2">
                        Share Settings
                    </div>
                    <v-divider class="mb-2"></v-divider>
        
                    <share-connections-editor
                        :match-uuid="matchUuid"
                        :video-uuid="clipUuid"
                        :game="game"
                        :no-clip="noClip"
                    >
                    </share-connections-editor>
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

        <v-snackbar
            v-model="errorShareProfile"
            :timeout="5000"
            color="error"
        >
            Failed to share/remove from your profile. Please try again!
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Component, {mixins} from 'vue-class-component'
import CommonComponent from '@client/vue/CommonComponent'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { SquadOvGames } from '@client/js/squadov/game'
import { StatPermission } from '@client/js/stats/statPrimitives'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import ShareConnectionsEditor from '@client/vue/utility/squadov/ShareConnectionsEditor.vue'
import { LinkShareData, MatchVideoSharePermissions, ShareToProfileData } from '@client/js/squadov/share'
import FacebookShareButton from '@client/vue/utility/squadov/share/FacebookShareButton.vue'
import TwitterShareButton from '@client/vue/utility/squadov/share/TwitterShareButton.vue'
import RedditShareButton from '@client/vue/utility/squadov/share/RedditShareButton.vue'
import { secondsToTimeString } from '@client/js/time'

@Component({
    components: {
        LoadingContainer,
        ShareConnectionsEditor,
        FacebookShareButton,
        TwitterShareButton,
        RedditShareButton
    }
})
export default class MatchShareButton extends mixins(CommonComponent) {
    @Prop()
    matchUuid!: string | undefined

    @Prop()
    clipUuid!: string | undefined

    @Prop()
    game!: SquadOvGames

    @Prop({required: true})
    permissions!: MatchVideoSharePermissions | null

    @Prop({type: Boolean, default: false})
    noClip!: boolean

    @Prop()
    graphqlStats!: StatPermission[]

    @Prop({required: true})
    fullPath!: string

    @Prop({default: 0})
    timestamp!: number

    showHideError: boolean = false
    showHideShare: boolean = false
    shareTimestamp: boolean = false

    isSharedPublic: boolean | null = null

    isSharedProfile: ShareToProfileData | null = null
    profileChangeInProgress: boolean = false
    errorShareProfile: boolean = false

    shareUrl: string | null = null
    creatingLink: boolean = false
    needConfirmDeleteLink: boolean = false
    deletingLink: boolean = false
    messages: string[] = []

    $refs!: {
        urlInput: any
    }

    get finalShareUrl(): string {
        if (!this.shareUrl) {
            return ''
        }

        let url = new URL(this.shareUrl)

        let params = new URLSearchParams(url.search)
        if (this.shareTimestamp) {
            params.append('t', secondsToTimeString(this.timestamp, true))
        }
        url.search = params.toString()

        return url.toString()
    }

    get warningText(): string {
        if (!!this.matchUuid) {
            return 'Warning: Everyone with this link will be able to view this match and the VODs you currently have share access to until you delete this link!'
        } else {
            return 'Warning: Everyone with this link will be able to view this clip until you delete this link!'
        }
    }

    toggleSharedToProfile(v: boolean) {
        if (!this.isSharedProfile?.canShare) {
            return
        }

        this.profileChangeInProgress = true
        let promise = v ? apiClient.shareMatchClipToProfile(this.matchUuid, this.clipUuid) : apiClient.deleteMatchClipFromProfile(this.matchUuid, this.clipUuid)
        
        promise.then(() => {
            if (!!this.isSharedProfile) {
                this.isSharedProfile.isShared = v
            }
        }).catch((err: any) => {
            console.error('Failed to toggle profile share: ', err)
            this.errorShareProfile = true
        }).finally(() => {
            this.profileChangeInProgress = true
        })
    }

    handlePublicLinkResponse(resp: LinkShareData) {
        this.isSharedPublic = resp.isLinkShared
        this.shareUrl = resp.shareUrl
    }

    generatePublicLink() {
        this.creatingLink = true
        
        let promise = !!this.matchUuid ?
            apiClient.createMatchShareUrl(this.matchUuid, this.fullPath, this.game, this.graphqlStats) :
            apiClient.createClipShareUrl(this.clipUuid!, this.fullPath)

        this.sendAnalyticsEvent(this.AnalyticsCategory.Share, this.AnalyticsAction.CreateSharePublic, !!this.matchUuid ? 'Match': 'Clip', 0)
        promise.then((resp: ApiData<LinkShareData>) => {
            this.handlePublicLinkResponse(resp.data)
        }).catch((err: any) => {
            this.showHideError = true
            console.error('Failed to create share URL: ', err)
        }).finally(() => {
            this.creatingLink = false
        })
    }

    deletePublicLink() {
        this.deletingLink = true

        this.sendAnalyticsEvent(this.AnalyticsCategory.Share, this.AnalyticsAction.DeleteSharePublic, !!this.matchUuid ? 'Match': 'Clip', 0)
        let promise = !!this.matchUuid ? apiClient.deleteMatchShareUrl(this.matchUuid) : apiClient.deleteClipShareUrl(this.clipUuid!)
        promise.then(() => {
            this.isSharedPublic = false
            this.shareUrl = null
        }).catch((err: any) => {
            this.showHideError = true
            console.error('Failed to delete share URL: ', err)
        }).finally(() => {
            this.deletingLink = false
        })
    }

    @Watch('matchUuid')
    @Watch('clipUuid')
    refreshLinkShareData() {
        this.isSharedPublic = null
        this.isSharedProfile = null
        this.shareUrl = null

        let promise = !!this.matchUuid ? apiClient.getMatchShareUrl(this.matchUuid) : apiClient.getClipShareUrl(this.clipUuid!)
        promise.then((resp: ApiData<LinkShareData>) => {
            this.handlePublicLinkResponse(resp.data)
        }).catch((err: any) => {
            this.showHideError = true
            console.error('Failed to get share URL: ', err)
        })

        apiClient.checkIfMatchClipSharedToProfile(this.matchUuid, this.clipUuid).then((resp: ApiData<ShareToProfileData>) => {
            this.isSharedProfile = resp.data
        }).catch((err: any) => {
            console.error('Failed to get shared to profile: ', err)
        })
    }

    onShow() {
        this.refreshLinkShareData()
        this.sendAnalyticsEvent(this.AnalyticsCategory.Share, this.AnalyticsAction.ShareMatchClip, !!this.matchUuid ? 'Match': 'Clip', 0)
        this.showHideShare = true
    }

    doCopy() {
        this.sendAnalyticsEvent(this.AnalyticsCategory.Share, this.AnalyticsAction.CopySharePublic, !!this.matchUuid ? 'Match': 'Clip', 0)

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