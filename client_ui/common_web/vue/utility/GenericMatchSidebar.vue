<template>
    <div>
        <v-tabs class="full-parent-height" grow centered>
            <template v-if="!hideEvents">
                <v-tab>
                    <v-tooltip bottom>
                        <template v-slot:activator="{ on, attrs }">
                            <v-icon v-on="on" v-bind="attrs">
                                mdi-map-marker
                            </v-icon>
                        </template>

                        Game Events
                    </v-tooltip>
                </v-tab>

                <v-tab-item>
                    <slot name="events"></slot>
                </v-tab-item>
            </template>

            <v-tab>
                <v-tooltip bottom>
                    <template v-slot:activator="{ on, attrs }">
                        <v-icon v-on="on" v-bind="attrs">
                            mdi-bookmark
                        </v-icon>
                    </template>

                    Bookmarks
                </v-tooltip>
            </v-tab>

            <v-tab-item>
                <div id="bookmark-container">
                    <template v-if="!!bookmarks && bookmarks.length > 0">
                        <div
                            v-for="(bm, idx) in sortedBookmarks"
                            :key="`bm-${idx}`"
                            class="d-flex justify-space-between align-center mx-4"
                        >
                            <template v-if="!editBookmark || editBookmark.eventId !== bm.eventId">
                                <div class="d-flex align-center">
                                    <div v-if="bm.userId === $store.state.currentUser.id" class="mr-2">
                                        <v-menu
                                            offset-y
                                        >
                                            <template v-slot:activator="{on}">
                                                <v-btn text icon v-on="on">
                                                    <v-icon>
                                                        mdi-dots-vertical
                                                    </v-icon>
                                                </v-btn>
                                            </template>

                                            <v-list dense>
                                                <v-list-item @click="editBookmark = cleanBookmarkFromJson(JSON.parse(JSON.stringify(bm)))" :disabled="!!editBookmark">
                                                    <v-list-item-title>
                                                        Edit
                                                    </v-list-item-title>
                                                </v-list-item>

                                                <v-list-item @click="deleteBookmark(bm.eventId)">
                                                    <v-list-item-title>
                                                        Delete
                                                    </v-list-item-title>
                                                </v-list-item>
                                            </v-list>
                                        </v-menu>
                                    </div>

                                    <div>
                                        {{ secondsToTimeString((bm.tm.getTime() - startTm.getTime()) / 1000.0, false) }}
                                    </div>
                                </div>

                                <div class="d-flex align-center">
                                    <v-icon class="mr-2">
                                        {{ convertBookmarkIcon(bm.icon) }}
                                    </v-icon>

                                    <div>
                                        <div class="font-weight-bold text-h6">
                                            {{ bm.label }}
                                        </div>

                                        <div class="text-subtitle-2">
                                            {{ bm.username }}
                                        </div>
                                    </div>
                                </div>

                                <v-btn
                                    outlined
                                    fab
                                    small
                                    color="white"
                                    @click="$emit('go-to-time', bm.tm)"
                                >
                                    <v-icon>mdi-play</v-icon>
                                </v-btn>
                            </template>

                            <template v-else>
                                <v-text-field
                                    class="flex-grow-0"
                                    :value="secondsToTimeString((editBookmark.tm.getTime() - startTm.getTime()) / 1000.0)"
                                    @change="changeEditBookmarkTime"
                                    hide-details
                                    dense
                                    solo
                                    single-line
                                    prepend-icon="mdi-alarm"
                                >
                                    <template v-slot:append>
                                        <v-btn icon @click="editBookmark.tm = currentTm" class="import-button">
                                            <v-icon>
                                                mdi-import
                                            </v-icon>
                                        </v-btn>
                                    </template>
                                </v-text-field>

                                <v-text-field
                                    v-model="editBookmark.label"
                                    hide-details
                                    outlined
                                    single-line
                                    prepend-icon="mdi-label"
                                >
                                </v-text-field>

                                <v-btn
                                    color="error"
                                    icon
                                    @click="editBookmark = null"
                                >
                                    <v-icon>
                                        mdi-close
                                    </v-icon>
                                </v-btn>

                                <v-btn
                                    color="success"
                                    icon
                                    @click="finishEditBookmark"
                                >
                                    <v-icon>
                                        mdi-check
                                    </v-icon>
                                </v-btn>
                            </template>
                        </div>
                    </template>

                    <v-row class="full-parent-height" justify="center" align="center" v-else>
                        <v-col cols="7">
                            <div class="text-h4 font-weight-bold" v-if="!!bookmarks">No bookmarks found (yet!).</div>
                            <v-progress-circular size="64" indeterminate v-else></v-progress-circular>
                        </v-col>
                    </v-row>
                </div>

                <div class="bookmark-footer">
                    <v-btn
                        id="bm-footer-button"
                        block
                        color="primary"
                        @click="createNewBookmark"
                        :loading="newBookmarkInProgress"
                        :disabled="!currentTm"
                    >
                        <v-icon>
                            mdi-plus
                        </v-icon>
                        New Bookmark
                    </v-btn>
                </div>
            </v-tab-item>
        </v-tabs>

        <v-snackbar
            v-model="errNewBookmark"
            :timeout="5000"
            color="error"
            absolute
            bottom
        >
            Failed to create a new bookmark. Please try again later.
        </v-snackbar>

        <v-snackbar
            v-model="errEditBookmark"
            :timeout="5000"
            color="error"
            absolute
            bottom
        >
            Failed to edit bookmark. Please try again later.
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { Bookmark, cleanBookmarkFromJson } from '@client/js/squadov/bookmarks'
import { apiClient, ApiData } from '@client/js/api'
import { secondsToTimeString, timeStringToMilliseconds } from '@client/js/time'

@Component
export default class GenericMatchSidebar extends Vue {
    secondsToTimeString = secondsToTimeString
    cleanBookmarkFromJson = cleanBookmarkFromJson

    @Prop({required: true})
    matchUuid!: string

    @Prop({type: Boolean, default: false})
    hideEvents!: boolean

    @Prop()
    currentTm!: Date | undefined | null

    @Prop()
    startTm!: Date

    bookmarks: Bookmark[] | null = null
    editBookmark: Bookmark | null = null

    errNewBookmark: boolean = false
    errDeleteBookmark: boolean = false
    errEditBookmark: boolean = false

    get sortedBookmarks(): Bookmark[] {
        if (!this.bookmarks) {
            return []
        }

        let arr = this.bookmarks
        arr.sort((a: Bookmark, b: Bookmark) => {
            return a.tm.getTime() - b.tm.getTime()
        })
        return arr
    }

    @Watch('matchUuid')
    refreshData() {
        this.bookmarks = null
        apiClient.getMatchAccessibleBookmarks(this.matchUuid).then((resp: ApiData<Bookmark[]>) => {
            this.bookmarks = resp.data
        }).catch((err: any) => {
            console.warn('Failed to load bookmarks: ', err)
        })
    }

    newBookmarkInProgress: boolean = false
    createNewBookmark() {
        if (!this.currentTm) {
            return
        }

        this.newBookmarkInProgress = true
        apiClient.createNewBookmark(this.matchUuid, this.currentTm).then(() => {
            this.refreshData()
        }).catch((err: any) => {
            console.warn('Failed to create new bookmark: ', err)
            this.errNewBookmark = true
        }).finally(() => {
            this.newBookmarkInProgress = false
        })
    }

    deleteBookmark(eventId: number) {
        apiClient.deleteBookmark(eventId).then(() => {
            if (!this.bookmarks) {
                return
            }

            this.bookmarks = this.bookmarks.filter((ele: Bookmark) => ele.eventId !== eventId)
        }).catch((err: any) => {
            console.warn('Failed to delete bookmark: ', err)
            this.errDeleteBookmark = true
        })
    }

    finishEditBookmark() {
        if (!this.editBookmark) {
            return
        }

        apiClient.editBookmark(this.editBookmark.eventId, this.editBookmark.label, this.editBookmark.tm).then(() => {
            if (!!this.bookmarks) {
                this.bookmarks = this.bookmarks.map((ele: Bookmark) => {
                    if (ele.eventId === this.editBookmark?.eventId) {
                        return cleanBookmarkFromJson(JSON.parse(JSON.stringify(this.editBookmark)))
                    } else {
                        return ele
                    }
                })
            }
            this.editBookmark = null
        }).catch((err: any) => {
            console.warn('Failed to edit bookmark: ', err)
            this.errEditBookmark = true
        })
    }

    mounted() {
        this.refreshData()
    }

    convertBookmarkIcon(input: string): string {
        switch (input) {
            default:
                return 'mdi-map-marker'
        }
    }

    changeEditBookmarkTime(s: string) {
        if (!this.editBookmark) {
            return
        }

        this.editBookmark.tm = new Date(this.startTm.getTime() + timeStringToMilliseconds(s))
    }
}

</script>

<style scoped>

#bookmark-container {
    height: calc(100% - 36px);
    max-height: calc(100% - 36px);
    overflow-y: auto;
}

>>>.v-tabs-items {
    height: calc(100% - 48px);
}

>>>.v-window__container {
    height: 100%;
}

>>>.v-window-item {
    height: 100%;
}

.bookmark-footer {
    position: relative;
    padding-bottom: 36px;
}

.bookmark-footer #bm-footer-button {
    position: absolute;
    bottom: 0px;
}

</style>