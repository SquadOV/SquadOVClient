<template>
    <div v-if="canDelete">
        <v-tooltip bottom>
            <template v-slot:activator="{on, attrs}">
                <v-btn
                    color="error"
                    icon
                    @click="showHideDeleteConfirm = true"
                    v-on="on"
                    v-bind="attrs"
                    :loading="loadingDelete || loadingLocalDelete"
                >
                    <v-icon>
                        mdi-delete
                    </v-icon>
                </v-btn>
            </template>

            Delete
        </v-tooltip>

        <!-- delete VOD button -->
        <v-dialog v-model="showHideDeleteConfirm" persistent max-width="40%">
            <v-card>
                <v-card-title>
                    Confirmation
                </v-card-title>
                <v-divider></v-divider>

                <v-card-text class="mt-4">
                    <div>
                        Are you sure you wish to delete your VOD? You and your squadmates will no longer be able to watch this match from your point of view.
                        This will delete the VOD from SquadOV's servers as well as from your local machine (if applicable).
                    </div>

                    <div class="mt-4">
                        Please type <span class="font-weight-bold">CONFIRM</span> (case-sensitive) to confirm this action.
                    </div>

                    <v-text-field
                        class="mt-4"
                        filled
                        label="Confirmation"
                        v-model="confirmationText"
                        hide-details
                    >
                    </v-text-field>
                </v-card-text>

                <v-card-actions>
                    <v-btn @click="hideDeleteConfirm">
                        Cancel
                    </v-btn>

                    <v-spacer></v-spacer>

                    <v-btn
                        color="error"
                        :loading="loadingDelete"
                        :disabled="confirmationText != 'CONFIRM'"
                        @click="deleteVod"
                    >
                        Delete
                    </v-btn>
                </v-card-actions>
            </v-card>
        </v-dialog>

        <v-snackbar
            v-model="deleteError"
            :timeout="5000"
            color="error"
        >
            Failed to delete the VOD, please try again later.
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Component, {mixins} from 'vue-class-component'
import CommonComponent from '@client/vue/CommonComponent'
import { Prop, Watch } from 'vue-property-decorator'
import { VodAssociation } from '@client/js/squadov/vod'
import { apiClient } from '@client/js/api'
import { VodRemoteControlContext } from '@client/js/vods/remote'

/// #if DESKTOP
import { ipcRenderer } from 'electron'
import { IpcResponse } from '@client/js/system/ipc'
/// #endif

@Component
export default class VodDeleteButton extends mixins(CommonComponent) {
    @Prop({ required: true })
    vod!: VodAssociation | null

    showHideDeleteConfirm: boolean = false
    loadingDelete: boolean = false
    loadingLocalDelete: boolean = false
    confirmationText: string = ''
    deleteError: boolean = false

    rcContext: VodRemoteControlContext | null = null

    hideDeleteConfirm() {
        this.showHideDeleteConfirm = false
        this.confirmationText = ''
    }

    onDeleteFinish(videoUuid: string) {
        this.hideDeleteConfirm()
        this.$emit('on-delete', videoUuid)
    }

    doDeleteVod(withLocal: boolean) {
        if (!this.vod) {
            return
        }

        this.loadingLocalDelete = withLocal
        this.sendAnalyticsEvent(this.AnalyticsCategory.MatchVod, this.AnalyticsAction.DeleteVod, '', 0)

        apiClient.deleteVod(this.vod.videoUuid).then(() => {
            if (!this.loadingLocalDelete) {
                this.onDeleteFinish(this.vod!.videoUuid)
            }
        }).catch((err: any) => {
            console.error('Failed to delete VOD: ', err)
            this.deleteError = true
        }).finally(() => {
            this.loadingDelete = false
        })

        if (!!this.rcContext && withLocal) {
            // Need a delay to give us time to release the file handle on the client side by destorying the player.
            this.rcContext.stopAndDestroy()
            setTimeout(() => {
                ipcRenderer.invoke('delete-vod-local', this.vod!.videoUuid).then((resp: IpcResponse<void>) => {
                    if (resp.success) {
                        if (!this.loadingDelete) {
                            this.onDeleteFinish(this.vod!.videoUuid)
                        }
                    } else {
                        this.deleteError = true
                    }
                }).catch((err: any) => {
                    console.error('Failed to delete local VOD: ', err)
                    this.deleteError = true
                }).finally(() => {
                    this.loadingLocalDelete = false
                })
            }, 1000)
        }
    }

    deleteVod() {
        if (!this.vod) {
            return
        }

        this.loadingDelete = true
///#if DESKTOP

        // We structure it this way so that we always check if the VOD exists locally first before doing any deletion.
        // This helps us maintain the logic where the deletes need to both finish before we call the onDeleteFinish function
        // otherwise we'd run into weird situations where the API server could finish the delete before the local service
        // even checks the VOD exists.
        ipcRenderer.invoke('check-vod-local', this.vod.videoUuid).then((resp: IpcResponse<string>) => {
            this.doDeleteVod(resp.success)
        }).catch((err: any) => {
            console.error('Failed to check for local VOD: ', err)
            this.doDeleteVod(false)
        })
///#else
        this.doDeleteVod(false)        
///#endif
    }

    get canDelete(): boolean {
        return !!this.vod && this.vod.userUuid === this.$store.state.currentUser.uuid
    }

    @Watch('vod')
    refreshRcContext() {
        if (!this.vod) {
            return
        }

///#if DESKTOP
        this.rcContext = new VodRemoteControlContext(this.vod.videoUuid)
///#endif
    }

    mounted() {
        this.refreshRcContext()
    }
}

</script>