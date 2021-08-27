<template>
    <div>
        <div class="d-flex align-center pa-4">
            <div class="text-h5 font-weight-bold">
                Local Storage Management
            </div>

            <v-spacer></v-spacer>

            <div>
                <div class="d-flex align-center justify-end">
                    <v-dialog persistent v-model="showDeleteConfirm" max-width="60%" v-if="inSelectMode">
                        <template v-slot:activator="{on, attrs}">
                            <v-btn
                                color="error"
                                icon
                                :disabled="selected.length === 0"
                                v-on="on"
                                v-bind="attrs"
                            >
                                <v-icon>
                                    mdi-delete
                                </v-icon>
                            </v-btn>
                        </template>

                        <v-card>
                            <v-card-title>
                                Are you sure?
                            </v-card-title>

                            <v-card-text>
                                Are you sure you want to delete {{ selected.length }} VODs from your local storage?
                                This will <b>NOT</b> delete the VOD from our servers.
                            </v-card-text>

                            <v-card-actions>
                                <v-btn
                                    color="error"
                                    @click="showDeleteConfirm = false"
                                >
                                    Cancel
                                </v-btn>

                                <v-spacer></v-spacer>

                                <v-btn
                                    color="success"
                                    @click="deleteLocalVods"
                                    :loading="deleteInProgress"
                                >
                                    Delete
                                </v-btn>
                            </v-card-actions>
                        </v-card>
                    </v-dialog>

                    <v-dialog persistent v-model="showUploadConfirm" max-width="60%" v-if="inSelectMode">
                        <template v-slot:activator="{on, attrs}">
                            <v-btn
                                color="accent"
                                icon
                                :disabled="selected.length === 0"
                                v-on="on"
                                v-bind="attrs"
                            >
                                <v-icon>
                                    mdi-upload
                                </v-icon>
                            </v-btn>
                        </template>

                        <v-card>
                            <v-card-title>
                                Are you sure?
                            </v-card-title>

                            <v-card-text>
                                Are you sure you want to upload {{ selected.length }} VODs from your local storage?
                                This may take a significant amount of time and bandwidth.
                            </v-card-text>

                            <v-card-actions>
                                <v-btn
                                    color="error"
                                    @click="showUploadConfirm = false"
                                >
                                    Cancel
                                </v-btn>

                                <v-spacer></v-spacer>

                                <v-btn
                                    color="success"
                                    @click="uploadLocalVods"
                                >
                                    Upload
                                </v-btn>
                            </v-card-actions>
                        </v-card>
                    </v-dialog>

                    <v-btn color="primary" v-if="inSelectMode" @click="toggleSelectAll">
                        {{ hasSelectedAll ? 'Select None' : 'Select All' }}
                    </v-btn>

                    <v-btn color="success" icon @click="inSelectMode = true" v-if="!inSelectMode">
                        <v-icon>
                            mdi-cursor-default
                        </v-icon>
                    </v-btn>

                    <v-btn icon @click="inSelectMode = false" v-else-if="inSelectMode">
                        <v-icon>
                            mdi-close
                        </v-icon>
                    </v-btn>

                    <v-btn color="warning" icon @click="onRequestChangeRecordingLocation">
                        <v-icon>
                            mdi-folder-open
                        </v-icon>
                    </v-btn>

                    <v-btn color="primary" icon :to="localSettingsTo">
                        <v-icon>
                            mdi-cog
                        </v-icon>
                    </v-btn>
                </div>

                <local-disk-space-usage-display>
                </local-disk-space-usage-display>
            </div>
        </div> 
        <v-divider class="my-2"></v-divider>

        <div class="mx-4">
            <v-list-item-group v-model="selected" multiple>
                <loading-container :is-loading="localVods === null">
                    <template v-slot:default="{ loading }">
                        <template v-if="!loading && !!localVods && localVods.length > 0">
                            <v-list-item
                                class="pa-0"
                                v-for="vod in localVods"
                                :key="vod"
                                :value="vod"
                            >
                                <template v-slot="{active}">
                                    <template v-if="!!vodToMatch[vod]">
                                        <div class="d-flex align-center full-width">
                                            <v-list-item-action v-if="inSelectMode">
                                                <v-checkbox :input-value="active"></v-checkbox>
                                            </v-list-item-action>

                                            <recent-match-display
                                                class="mb-4 full-width"
                                                :match="vodToMatch[vod]"
                                                :disable-click="inSelectMode"
                                                use-local-vod-preview
                                                disable-mini
                                                show-upload-progress
                                                :ref="vod"
                                            >
                                            </recent-match-display>
                                        </div>
                                    </template>
                                </template>
                                
                            </v-list-item>
                        </template>
                    </template>
                </loading-container>
            </v-list-item-group>

            <v-btn
                v-if="hasMore"
                color="primary"
                block
                @click="loadMoreData"  
            >
                Load More
            </v-btn>
        </div>

        <v-snackbar
            v-model="deleteError"
            :timeout="5000"
            color="error"
        >
            Failed to delete the VOD(s), please submit a bug report.
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import LocalDiskSpaceUsageDisplay from '@client/vue/utility/squadov/local/LocalDiskSpaceUsageDisplay.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import RecentMatchDisplay from '@client/vue/utility/RecentMatchDisplay.vue'
import { apiClient, ApiData } from '@client/js/api'
import { RecentMatch } from '@client/js/squadov/recentMatch'

/// #if DESKTOP
import { shell, ipcRenderer } from 'electron'
import { IpcResponse } from '@client/js/system/ipc'
/// #endif

import { SettingsPageId } from '@client/js/pages'
import { LocalVodsDto } from '@client/js/local/types'
import { manager } from '@client/js/vods/local_manager'

const MAX_VODS_PER_REQUEST = 10

@Component({
    components: {
        LocalDiskSpaceUsageDisplay,
        LoadingContainer,
        RecentMatchDisplay,
    }
})
export default class LocalStorageManager extends Vue {
    localVods: string[] | null = null
    vodToMatch: { [uuid: string]: RecentMatch | undefined } = {}

    start: number = 0
    end: number = MAX_VODS_PER_REQUEST
    hasMore: boolean = false

    inSelectMode: boolean = false
    selected: string[] = []

    showDeleteConfirm: boolean = false
    deleteInProgress: boolean = false
    deleteError: boolean = false

    showUploadConfirm: boolean = false

    get hasSelectedAll(): boolean {
        if (!this.localVods) {
            return false
        }

        return this.selected.length === this.localVods.length
    }

    toggleSelectAll() {
        if (!this.localVods) {
            return
        }

        if (this.hasSelectedAll) {
            this.selected = []
        } else {
            this.selected = [...this.localVods]
        }
    }

    deleteLocalVods() {
        this.deleteInProgress = true
///#if DESKTOP
        for (let v of this.selected) {
            //@ts-ignore
            let ele: RecentMatchDisplay = this.$refs[v][0]
            ele.destroyPreview()
        }

        setTimeout(() => {
            Promise.all(this.selected.map((ele: string) => {
                return ipcRenderer.invoke('delete-vod-local', ele)
            })).then((resp: IpcResponse<void>[]) => {
                if (resp.every((ele: IpcResponse<void>) => ele.success)) {
                    let selectedSet = new Set(this.selected)

                    if (!!this.localVods) {
                        this.localVods = this.localVods.filter((ele: string) => !selectedSet.has(ele))
                    }
                    
                    this.selected = []
                    this.showDeleteConfirm = false
                } else {
                    this.deleteError = true
                }
            }).catch((err: any) => {
                console.error('Failed to delete local VOD: ', err)
                this.deleteError = true
            }).finally(() => {
                this.deleteInProgress = false
            })
        }, 1000)
///#endif
    }

    uploadLocalVods() {
        this.showUploadConfirm = false
        this.selected.forEach((ele: string) => manager.enqueueUpload(ele))
        this.selected = []
    }

    @Watch('localVods')
    syncMatches() {
        if (!this.localVods) {
            return
        }

        for (let vod of this.localVods) {
            if (vod in this.vodToMatch) {
                continue
            }

            apiClient.getMatchDataFromVideoUuid(vod).then((resp: ApiData<RecentMatch>) => {
                Vue.set(this.vodToMatch, vod, resp.data)
            }).catch((err: any) => {
                console.error('Failed to get match data for video: ', err)
            })
        }
    }

    get localSettingsTo(): any {
        return {
            name: SettingsPageId,
            query: {
                inputTab: 3,
            }
        }
    }

    get localRecordingLocation(): string {
        return this.$store.state.settings.record.localRecordingLocation
    }

    onRequestChangeRecordingLocation() {
///#if DESKTOP
        shell.openPath(this.localRecordingLocation)
///#endif
    }

    refreshData() {
        this.localVods = null
        this.start = 0
        this.end = MAX_VODS_PER_REQUEST
        this.loadMoreData()
    }

    loadMoreData() {
        apiClient.listLocalVods(this.start, this.end).then((resp: ApiData<LocalVodsDto>) => {
            if (!this.localVods) {
                this.localVods = []
            }

            this.localVods.push(...resp.data.data)
            this.start += resp.data.data.length
            this.end = this.start + MAX_VODS_PER_REQUEST
            this.hasMore = (resp.data.data.length >= MAX_VODS_PER_REQUEST)
        }).catch((err: any) => {
            console.error('Failed to list local VODs: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>