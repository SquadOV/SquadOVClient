<template>
    <div class="full-width pa-4">
        <div class="d-flex align-center">
            <div class="text-h6 font-weight-bold">
                {{ title }}
            </div>

            <v-btn
                class="ml-2"
                icon
                @click="refreshClips"
            >
                <v-icon>
                    mdi-refresh
                </v-icon>
            </v-btn>

            <v-spacer></v-spacer>

            <template v-if="!inSelectMode">
                <v-btn
                    color="primary"
                    @click="inSelectMode = true"
                >
                    Select
                </v-btn>
            </template>

            <template v-else>
                <v-dialog persistent v-model="showHideDelete" max-width="60%">
                    <template v-slot:activator="{on, attrs}">
                        <v-btn
                            icon
                            color="error"
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
                            Are you sure you want to delete {{ selected.length }} clips?
                            This action can not be undone.
                        </v-card-text>

                        <v-card-actions>
                            <v-btn
                                color="error"
                                @click="showHideDelete = false"
                            >
                                Cancel
                            </v-btn>

                            <v-spacer></v-spacer>

                            <v-btn
                                color="success"
                                @click="deleteSelectedClips"
                                :loading="deleteInProgress"
                            >
                                Delete
                            </v-btn>
                        </v-card-actions>
                    </v-card>
                </v-dialog>

                <v-btn
                    @click="toggleSelected"
                    outlined
                    class="ml-2"
                >
                    <v-checkbox
                        :input-value="allSelected"
                        readonly
                    >
                    </v-checkbox>

                    {{ selected.length }} Selected
                </v-btn>

                <v-btn
                    class="ml-2"
                    icon
                    @click="closeSelect"
                    color="warning"
                >
                    <v-icon>
                        mdi-close
                    </v-icon>
                </v-btn>
            </template>
        </div>
        <v-divider class="my-2"></v-divider>
        <recent-match-filters-ui
            v-model="filters"
            :disable-squads="isUserLocked"
            :disable-users="isUserLocked"
        ></recent-match-filters-ui>

        <loading-container :is-loading="!clips">
            <template v-slot:default="{ loading }">
                <v-list-item-group v-model="selected" multiple>
                    <v-container fluid v-if="!loading">
                        <v-row v-if="displayClips.length > 0">
                            <v-col cols="3" v-for="(c, idx) in displayClips" :key="idx">
                                <v-list-item class="pa-0" :value="c.clip.videoUuid">
                                    <template v-slot="{active}">
                                        <div class="d-flex align-center full-width">
                                            <v-list-item-action v-if="inSelectMode">
                                                <v-checkbox :input-value="active"></v-checkbox>
                                            </v-list-item-action>

                                            <mini-clip-preview
                                                class="flex-grow-1"
                                                :clip="c"
                                                :disable-click="inSelectMode"
                                            >
                                            </mini-clip-preview>
                                        </div>
                                    </template>
                                </v-list-item>
                            </v-col>
                        </v-row>

                        <div class="pa-4 text-h6 font-weight-bold" v-else>
                            No clips found.
                        </div>

                        <v-btn
                            v-if="hasNext"
                            color="primary"
                            block
                            @click="loadMoreData"  
                        >
                            Load More
                        </v-btn>
                    </v-container>
                </v-list-item-group>
            </template>
        </loading-container>

        <v-snackbar
            v-model="showHideDeleteError"
            :timeout="5000"
            color="error"
        >
            Failed to delete clips. Please try again.
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { VodClip } from '@client/js/squadov/vod'
import { apiClient, HalResponse, ApiData } from '@client/js/api'
import { RecentMatchFilters, createEmptyRecentMatchFilters } from '@client/js/squadov/recentMatch'
import MiniClipPreview from '@client/vue/utility/vods/MiniClipPreview.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import RecentMatchFiltersUi from '@client/vue/utility/RecentMatchFiltersUi.vue'

const maxClipsPerRequest: number = 20

@Component({
    components: {
        MiniClipPreview,
        LoadingContainer,
        RecentMatchFiltersUi
    }
})
export default class ClipLibrary extends Vue {
    @Prop({default: 'Clip Library'})
    title!: string

    @Prop()
    matchUuid!: string | undefined

    @Prop()
    userId!: number | undefined

    @Prop({type: Boolean, default: false})
    onlyFavorite!: boolean

    @Prop({type: Boolean, default: false})
    onlyWatchlist!: boolean

    clips: VodClip[] | null = null
    lastIndex: number = 0
    nextLink: string | null = null
    
    filters: RecentMatchFilters = createEmptyRecentMatchFilters()

    inSelectMode: boolean = false
    selected: string[] = []
    deleteInProgress: boolean = false
    showHideDelete: boolean = false
    showHideDeleteError: boolean = false

    get isUserLocked(): boolean {
        return this.userId !== undefined
    }

    get hasNext() : boolean {
        return !!this.nextLink
    }

    get selectableClips(): VodClip[] {
        if (!this.clips) {
            return []
        }
        return this.clips.filter((ele: VodClip) => {
            return ele.clip.userUuid === this.$store.state.currentUser.uuid
        })
    }

    get allSelected(): boolean {
        return this.selectableClips.length === this.selected.length
    }

    get displayClips(): VodClip[] {
        if (!this.clips) {
            return []
        }

        if (this.inSelectMode) {
            return this.selectableClips
        } else {
            return this.clips
        }
    }

    toggleSelected() {
        if (this.allSelected) {
            this.selected = []
        } else {
            this.selected = this.selectableClips.map((ele: VodClip) => ele.clip.videoUuid)
        }
    }

    deleteSelectedClips() {
        if (this.selected.length === 0) {
            return
        }

        this.deleteInProgress = true
        apiClient.deleteClips(this.selected).then(() => {
            this.refreshClips()
            this.showHideDelete = false
        }).catch((err: any) => {
            console.error('Failed to delete clips: ', err)
            this.showHideDeleteError = true
        }).finally(() => {
            this.deleteInProgress = false
        })
    }

    closeSelect() {
        this.inSelectMode = false
        this.selected = []
    }

    get finalFilters(): RecentMatchFilters {
        let filters: RecentMatchFilters = JSON.parse(JSON.stringify(this.filters))

        if (this.userId !== undefined && !this.onlyFavorite && !this.onlyWatchlist) {
            filters.users = [this.userId]
        }

        filters.onlyFavorite = this.onlyFavorite
        filters.onlyWatchlist = this.onlyWatchlist
        return filters
    }

    @Watch('finalFilters', { deep: true })
    @Watch('matchUuid')
    refreshClips() {
        this.clips = null
        this.nextLink = null
        this.lastIndex = 0
        this.selected = []
        this.loadMoreData()
    }

    loadMoreData() {
        if (!!this.clips && !this.nextLink) {
            return
        }

        apiClient.listClips({
            next: this.nextLink,
            matchUuid: this.matchUuid,
            start: this.lastIndex,
            end: this.lastIndex + maxClipsPerRequest,
            filters: this.finalFilters,
        }).then((resp : ApiData<HalResponse<VodClip[]>>) => {
            if (!this.clips) {
                this.clips = resp.data.data
            } else {
                this.clips.push(...resp.data.data)
            }
            this.lastIndex += resp.data.data.length
            if ("next" in resp.data._links) {
                this.nextLink = resp.data._links["next"].href
            } else {
                this.nextLink = null
            }
        }).catch((err : any) => {
            console.error('Failed to list clips: ' + err);
        })
    }

    mounted() {
        this.refreshClips()
    }
}

</script>