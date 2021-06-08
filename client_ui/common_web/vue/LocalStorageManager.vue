<template>
    <div>
        <div class="d-flex align-center pa-4">
            <div class="text-h5 font-weight-bold">
                Local Storage Management
            </div>

            <v-spacer></v-spacer>

            <div>
                <div class="d-flex align-center justify-end">
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
import { shell } from 'electron'
/// #endif

import { SettingsPageId } from '@client/js/pages'
import { LocalVodsDto } from '@client/js/local/types'

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
    hasMore: boolean = true

    inSelectMode: boolean = false
    selected: string[] = []

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
                console.log('Failed to get match data for video: ', err)
            })
        }
    }

    get localSettingsTo(): any {
        return {
            name: SettingsPageId,
            query: {
                inputTab: 2
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
            console.log('Failed to list local VODs: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>