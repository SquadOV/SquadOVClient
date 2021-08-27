<template>
    <v-card>
        <v-card-title>
            Invite friends to your squad!
        </v-card-title>
        <v-divider></v-divider>

        <v-tabs>
            <v-tab>
                Links
            </v-tab>

            <v-tab-item>
                <div class="mx-4 mt-4 mb-2" v-if="!linkToModify">
                    <loading-container :is-loading="inviteLinks === null">
                        <template v-slot:default="{ loading }">
                            <div v-if="!loading">
                                <div
                                    class="d-flex align-center mb-2"
                                    v-for="(link, idx) in sortedInviteLinks"
                                    :key="idx"
                                >
                                    <v-text-field
                                        :value="link.link"
                                        hide-details
                                        single-line
                                        outlined
                                        dense
                                        readonly
                                        :ref="link.id"
                                    >
                                        <template v-slot:append>        
                                            <div class="text-overline">
                                                Uses: {{ link.useCount }}
                                            </div>
                                        </template>
                                    </v-text-field>

                                    <v-btn
                                        class="ml-2"
                                        icon
                                        color="success"
                                        @click="copyLink(link.id)"
                                    >
                                        <v-icon>
                                            mdi-content-copy
                                        </v-icon>
                                    </v-btn>

                                    <v-btn
                                        class="ml-2"
                                        icon
                                        @click="editLink(link)"
                                    >
                                        <v-icon>
                                            mdi-cog
                                        </v-icon>
                                    </v-btn>
                                </div>

                                <v-btn
                                    color="primary"
                                    block
                                    @click="newInviteLink"
                                    :loading="newLinkInProgress"
                                >
                                    <v-icon> 
                                        mdi-plus
                                    </v-icon>
                                    
                                    Create New Link
                                </v-btn>
                            </div>
                        </template>
                    </loading-container>
                </div>

                <div class="mx-4 mt-4 mb-2" v-else>
                    <div class="d-flex align-center">
                        <v-text-field
                            label="Link"
                            :value="linkToModify.link"
                            hide-details
                            single-line
                            outlined
                            dense
                            readonly
                        >
                            <template v-slot:append>        
                                <div class="text-overline">
                                    Uses: {{ linkToModify.useCount }}
                                </div>
                            </template>
                        </v-text-field>

                        <template v-if="needDeleteConfirm">
                            <v-btn
                                color="error"
                                icon
                                @click="needDeleteConfirm = false"
                            >
                                <v-icon>
                                    mdi-close
                                </v-icon>
                            </v-btn>

                            <v-btn
                                color="success"
                                icon
                                @click="deleteLink(linkToModify.id)"
                                :loading="deleteInProgress"
                            >
                                <v-icon>
                                    mdi-check
                                </v-icon>
                            </v-btn>
                        </template>

                        <v-btn
                            v-else
                            color="error"
                            icon
                            @click="needDeleteConfirm = true"
                        >
                            <v-icon>
                                mdi-delete
                            </v-icon>
                        </v-btn>
                    </div>

                    <div class="d-flex align-center mt-2">
                        <v-select
                            class="mt-2"
                            label="Expire After"
                            outlined
                            dense
                            hide-details
                            v-model="newLinkExpirationMs"
                            :items="expirationItems"
                        >
                        </v-select>

                        <div class="text-caption ml-2">
                            <div class="font-weight-bold">
                                Current Expiration:
                            </div>

                            <div v-if="!linkToModify.expireTime">
                                Never
                            </div>

                            <div v-else>
                                {{ standardFormatTime(linkToModify.expireTime) }}
                            </div>
                        </div>
                    </div>

                    <v-select
                        class="mt-2"
                        label="Max Uses"
                        v-model="linkToModify.maxUses"
                        :items="maxUseItems"
                        outlined
                        dense
                        hide-details
                    >
                    </v-select>

                    <div class="d-flex align-center mt-2">
                        <v-btn
                            color="error"
                            @click="cancelLinkChanges"
                        >
                            Cancel
                        </v-btn>

                        <v-spacer></v-spacer>

                        <v-btn
                            color="success"
                            @click="saveLinkChanges"
                            :loading="modifyingLink"
                        >
                            Save
                        </v-btn>
                    </div>
                </div>
            </v-tab-item>

            <v-tab>
                Direct
            </v-tab>

            <v-tab-item>
                <div class="mx-4 mt-4 mb-2">
                    <v-combobox
                        v-model="inviteUsernames"
                        clearable
                        filled
                        chips
                        label="Usernames"
                        multiple
                        deletable-chips
                        hide-details
                    >
                    </v-combobox>

                    <div class="d-flex my-4 justify-center align-center">
                        OR
                    </div>

                    <v-combobox
                        v-model="inviteEmails"
                        clearable
                        filled
                        chips
                        label="Emails"
                        multiple
                        deletable-chips
                        hide-details
                    >
                    </v-combobox>

                    <v-btn
                        block
                        class="mt-2"
                        color="success"
                        :loading="invitePending"
                        @click="sendInvite"
                        :disabled="totalInvites == 0"
                    >
                        Send
                    </v-btn>
                </div>
            </v-tab-item>
        </v-tabs>

        <v-snackbar
            v-model="inviteLinkError"
            :timeout="5000"
            color="error"
        >
            Failed to create squad invite link.
        </v-snackbar>

        <v-snackbar
            v-model="showError"
            :timeout="5000"
            color="error"
        >
            Failed to send squad invites. Please try again.
        </v-snackbar>

        <v-snackbar
            v-model="modifyError"
            :timeout="5000"
            color="error"
        >
            Failed to modify squad invite link. Please try again.
        </v-snackbar>

        <v-snackbar
            v-model="deleteError"
            :timeout="5000"
            color="error"
        >
            Failed to delete squad invite link. Please try again.
        </v-snackbar>

        <v-snackbar
            v-model="successCopy"
            :timeout="3000"
            color="success"
        >
            Successfully copied invite link.
        </v-snackbar>
    </v-card>
</template>

<script lang="ts">

import Component, {mixins} from 'vue-class-component'
import CommonComponent from '@client/vue/CommonComponent'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { Prop } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api' 
import { AnalyticsAction, AnalyticsCategory } from '@client/js/analytics/events'
import { cleanSquadInviteLinkFromJson, SquadInviteLink } from '@client/js/squadov/squad'
import { standardFormatTime } from '@client/js/time'

@Component({
    components: {
        LoadingContainer
    }
})
export default class SquadInviteCreateCard extends mixins(CommonComponent) {
    standardFormatTime = standardFormatTime

    @Prop({required: true})
    squadId!: number

    invitePending: boolean = false
    inviteUsernames: string[] = []
    inviteEmails: string[] = []

    inviteLinks: SquadInviteLink[] | null = null

    linkToModify: SquadInviteLink | null = null
    modifyingLink: boolean = false
    needDeleteConfirm: boolean = false
    deleteInProgress: boolean = false

    // For this, 0 = no limit, null = no change
    newLinkExpirationMs: number | null = null

    newLinkInProgress: boolean = false
    inviteLinkError: boolean = false

    showSuccess: boolean = false
    showError: boolean = false
    successCopy: boolean = false
    modifyError: boolean = false
    deleteError: boolean = false

    get expirationItems(): any[] {
        return [
            {
                text: '30 minutes',
                value: 30 * 60 * 1000,
            },
            {
                text: '1 hour',
                value: 60 * 60 * 1000,
            },
            {
                text: '12 hours',
                value: 12 * 60 * 60 * 1000,
            },
            {
                text: '1 day',
                value: 24 * 60 * 60 * 1000,
            },
            {
                text: '7 days',
                value: 7 * 24 * 60 * 60 * 1000,
            },
            {
                text: 'Never',
                value: 0,
            },
        ]
    }

    get maxUseItems(): any[] {
        return [
            {
                text: 'No limit',
                value: null,
            },
            {
                text: '1 use',
                value: 1,
            },
            {
                text: '5 uses',
                value: 5,
            },
            {
                text: '10 uses',
                value: 10,
            },
            {
                text: '25 uses',
                value: 25,
            },
            {
                text: '50 uses',
                value: 50,
            },
            {
                text: '100 uses',
                value: 100,
            },
        ]
    }

    get sortedInviteLinks(): SquadInviteLink[] {
        if (!this.inviteLinks) {
            return []
        }

        return [...this.inviteLinks].sort((a: SquadInviteLink, b: SquadInviteLink) => {
            return b.useCount - a.useCount
        })
    }

    get totalInvites(): number {
        return this.inviteUsernames.length + this.inviteEmails.length
    }

    cancelLinkChanges() {
        this.linkToModify = null
        this.newLinkExpirationMs = null
    }

    deleteLink(id: string) {
        this.deleteInProgress = true
        apiClient.deleteSquadInviteLink(this.squadId, this.$store.state.currentUser.id, id).then(() => {
            this.deleteLinkFromLocal(id)
            this.cancelLinkChanges()
        }).catch((err: any) => {
            console.error('Failed to delete link: ', err)
            this.deleteError = true
        }).finally(() => {
            this.deleteInProgress = false
        })
    }

    deleteLinkFromLocal(id: string) {
        if (!this.inviteLinks) {
            return
        }
        let idx = this.inviteLinks.findIndex((ele: SquadInviteLink) => ele.id === id)
        if (idx == -1) {
            return
        }

        this.inviteLinks.splice(idx, 1)
    }

    saveLinkChanges() {
        if (!this.linkToModify) {
            return
        }

        this.modifyingLink = true
        if (this.newLinkExpirationMs !== null) {
            if (this.newLinkExpirationMs == 0) {
                this.linkToModify.expireTime = null
            } else {
                this.linkToModify.expireTime = new Date(new Date().getTime() + this.newLinkExpirationMs)
            }
        }

        let newLink = this.linkToModify
        apiClient.modifySquadInviteLink(newLink).then(() => {
            if (!this.inviteLinks || !newLink) {
                return
            }

            this.deleteLinkFromLocal(newLink.id)
            this.inviteLinks.push(newLink)
            this.cancelLinkChanges()
        }).catch((err: any) => {
            console.error('Failed to modify squad invite link: ', err)
            this.modifyError = true
        }).finally(() => {
            this.modifyingLink = false
        })
    }

    editLink(link: SquadInviteLink) {
        this.linkToModify = cleanSquadInviteLinkFromJson(JSON.parse(JSON.stringify(link)))
    }

    copyLink(id: string) {
        //@ts-ignore
        let inputRef: Vue[] | undefined = this.$refs[id]
        if (!inputRef) {
            return
        }

        let inputEle = inputRef[0].$el.querySelector('input')
        if (!inputEle){ 
            return
        }

        inputEle.select()
        document.execCommand('copy')
        this.successCopy = true
    }

    newInviteLink() {
        this.newLinkInProgress = true
        apiClient.createSquadInviteLinkForUser(this.squadId, this.$store.state.currentUser.id).then((resp: ApiData<SquadInviteLink>) => {
            if (!!this.inviteLinks) {
                this.inviteLinks.push(resp.data)
            }
        }).catch((err: any) => {
            console.error('Failed to create squad invite link: ', err)
            this.inviteLinkError = true
        }).finally(() => {
            this.newLinkInProgress = false
        })
    }

    refreshLinks() {
        apiClient.getSquadInviteLinksForUser(this.squadId, this.$store.state.currentUser.id).then((resp: ApiData<SquadInviteLink[]>) => {
            this.inviteLinks = resp.data
        }).catch((err: any) => {
            console.error('Failed to get squad invite links: ', err)
        })
    }

    mounted() {
        this.refreshLinks()
    }

    clear() {
        this.inviteUsernames = []
        this.inviteEmails = []
    }

    sendInvite() {
        this.invitePending = true
        this.sendAnalyticsEvent(AnalyticsCategory.Button, AnalyticsAction.InviteSquad, '', 0)
        
        apiClient.sendSquadInvite(this.squadId, this.inviteUsernames, this.inviteEmails).then(() => {
            this.showSuccess = true
            this.$emit('on-send-invite')
            this.clear()
        }).catch((err: any) => {
            console.error('Failed to send squad invites: ', err)
            this.showError = true
        }).finally(() => {
            this.invitePending = false
        })
    }
}

</script>