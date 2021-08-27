<template>
    <v-container fluid>
        <v-row justify="center">
            <v-col cols="3" v-if="isLoggedIn">
                <div class="d-flex align-center">
                    <span class="font-weight-bold">Communities</span>
                    <v-spacer></v-spacer>
                    <v-btn
                        color="primary"
                        small
                        icon
                        @click="refreshCommunities"
                        :loading="refreshPending"
                    >
                        <v-icon>
                            mdi-refresh
                        </v-icon>
                    </v-btn>
                </div>

                <div>
                    <v-form @submit.prevent="search">
                        <v-text-field
                            v-model="searchQuery"
                            dense
                            hide-details
                            outlined
                            label="Search"
                            single-line
                            placeholder="Search for communities"
                            prepend-icon="mdi-magnify"
                        >
                        </v-text-field>
                    </v-form>
                </div>

                <v-list>
                    <v-list-item exact :to="feedTo">
                        <v-list-item-icon>
                            <v-icon>
                                mdi-rss
                            </v-icon>
                        </v-list-item-icon>

                        <v-list-item-content>
                            Your Feed
                        </v-list-item-content>
                    </v-list-item>

                    <v-list-item exact :to="discoverTo">
                        <v-list-item-icon>
                            <v-icon>
                                mdi-compass
                            </v-icon>
                        </v-list-item-icon>

                        <v-list-item-content>
                            Discover
                        </v-list-item-content>
                    </v-list-item>
                </v-list>

                <v-dialog
                    v-model="showHideCreate"
                    persistent
                    max-width="60%"
                >
                    <template v-slot:activator="{on, attrs}">
                        <v-btn
                            v-bind="attrs"
                            v-on="on"
                            dense
                            block
                            color="primary"
                        >
                            <v-icon>
                                mdi-plus
                            </v-icon>

                            Create New Community
                        </v-btn>
                    </template>

                    <v-card>
                        <v-card-title>
                            New Community
                        </v-card-title>
                        <v-divider></v-divider>

                        <div class="pa-4">
                            <v-form v-model="newValid">
                                <v-text-field
                                    v-model="newCommunity.name"
                                    filled
                                    label="Name"
                                    :rules="nameRules"
                                >
                                </v-text-field>

                                <v-text-field
                                    v-model="newCommunity.slug"
                                    filled
                                    label="URL"
                                    :rules="slugRules"
                                >
                                </v-text-field>

                                <div class="text-caption">
                                    {{ `You will be able to access the community at the URL: https://app.squadov.gg/c/${newCommunity.slug}.` }}
                                </div>

                                <div class="d-flex my-2">
                                    <community-security-level-chooser
                                        class="px-1"
                                        v-model="newCommunity.securityLevel"
                                        style="width: 50%; max-width: 50%;"
                                    >
                                    </community-security-level-chooser>

                                    <div
                                        class="px-1"
                                        style="width: 50%; max-width: 50%;"
                                        v-if="newCommunity.securityLevel == CommunitySecurityLevel.Private"
                                    >
                                        <v-checkbox
                                            v-model="newCommunity.requiresSubscription"
                                            dense
                                            hide-details
                                            label="Requires Subscription"
                                        >
                                        </v-checkbox>

                                        <v-checkbox
                                            v-if="newCommunity.requiresSubscription"
                                            :input-value="newCommunity.allowTwitchSub"
                                            dense
                                            hide-details
                                            readonly
                                            label="Allow Twitch Sub"
                                        >
                                        </v-checkbox>
                                    </div>
                                </div>
                            </v-form>
                        </div>

                        <v-divider class="my-1"></v-divider>

                        <v-card-actions>
                            <v-btn
                                @click="showHideCreate = false"
                                color="error"
                            >
                                Cancel
                            </v-btn>
                            
                            <v-spacer></v-spacer>

                            <v-btn
                                @click="create"
                                color="success"
                                :loading="createPending"
                                :disabled="!newValid"
                            >
                                Create
                            </v-btn>
                        </v-card-actions>
                    </v-card>
                </v-dialog>

                <v-divider class="my-1"></v-divider>

                <div class="my-1">
                    <div class="font-weight-bold">
                        Communities you Manage
                    </div>

                    <loading-container :is-loading="!myCommunities">
                        <template v-slot:default="{ loading }">
                            <v-list v-if="!loading">
                                <community-list-item
                                    v-for="(comm, idx) in managedCommunities"
                                    :community="comm"
                                    :key="`manage-${idx}`"
                                >
                                </community-list-item>
                            </v-list>
                        </template>
                    </loading-container>
                </div>

                <v-divider class="my-1"></v-divider>

                <div class="my-1">
                    <div class="font-weight-bold">
                        Communities you've Joined
                    </div>

                    <loading-container :is-loading="!myCommunities">
                        <template v-slot:default="{ loading }">
                            <v-list v-if="!loading">
                                <community-list-item
                                    v-for="(comm, idx) in joinedCommunities"
                                    :community="comm"
                                    :key="`join-${idx}`"
                                >
                                </community-list-item>
                            </v-list>
                        </template>
                    </loading-container>
                </div>
            </v-col>

            <v-col cols="9">
                <keep-alive :max="3">
                    <router-view :key="routerKey"></router-view>
                </keep-alive>
            </v-col>
        </v-row>

        <v-snackbar
            v-model="createDupError"
            :timeout="5000"
            color="error"
        >
            A community with that name and/or URL already exists. Please select another.
        </v-snackbar>

        <v-snackbar
            v-model="createError"
            :timeout="5000"
            color="error"
        >
            Failed to create community. Please try again later.
        </v-snackbar>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import CommunityListItem from '@client/vue/utility/community/CommunityListItem.vue'
import CommunitySecurityLevelChooser from '@client/vue/utility/community/CommunitySecurityLevelChooser.vue'

import { apiClient, ApiData } from '@client/js/api'
import { CommunityDiscoverPageId, CommunityHomePageId } from '@client/js/pages'
import { SquadOvCommunity, createDefaultCommunity, CommunitySecurityLevel } from '@client/js/squadov/community'
import { routeLevelToKey } from '@client/js/routes'

@Component({
    components: {
        LoadingContainer,
        CommunityListItem,
        CommunitySecurityLevelChooser,
    }
})
export default class Community extends Vue {
    CommunitySecurityLevel = CommunitySecurityLevel

    searchQuery: string = ''
    myCommunities: SquadOvCommunity[] | null = null

    get sortedCommunities(): SquadOvCommunity[] {
        if (!this.myCommunities) {
            return []
        }
        return this.myCommunities.sort((a: SquadOvCommunity, b: SquadOvCommunity) => {
            if (a.name < b.name) {
                return -1
            } else if (a.name > b.name) {
                return 1
            } else {
                return 0
            }
        })
    }

    get managedCommunities(): SquadOvCommunity[] {
        return this.sortedCommunities.filter((ele: SquadOvCommunity) => {
            return ele.creatorUserId === this.$store.state.currentUser.id
        })
    }

    get joinedCommunities(): SquadOvCommunity[] {
        return this.sortedCommunities.filter((ele: SquadOvCommunity) => {
            return ele.creatorUserId !== this.$store.state.currentUser.id
        })
    }

    search() {
        let query = this.searchQuery.trim()
        if (query === '') {
            return
        }

        this.$router.push({
            name: CommunityDiscoverPageId,
            query: {
                search: query,
            }
        })
    }

    get routerKey(): string {
        let key = routeLevelToKey(this.$route, 2)
        return `${key}`
    }

    refreshPending: boolean = false
    refreshCommunities() {
        this.refreshPending = true
        apiClient.listCommunities({
            onlyMe: true
        }).then((resp: ApiData<SquadOvCommunity[]>) => {
            this.myCommunities = resp.data
        }).catch((err: any) => {
            console.error('Failed to list my communities: ', err)
        }).finally(() => {
            this.refreshPending = false
        })
    }

    get isLoggedIn(): boolean {
        return !!this.$store.state.currentUser
    }

    get feedTo(): any {
        return {
            name: CommunityHomePageId
        }
    }

    get discoverTo(): any {
        return {
            name: CommunityDiscoverPageId
        }
    }

    showHideCreate: boolean = false
    createPending: boolean = false
    newCommunity: SquadOvCommunity = createDefaultCommunity()
    newValid: boolean = false
    createError: boolean = false
    createDupError: boolean = false

    create() {
        if (!this.newValid) {
            return
        }

        this.createPending = true
        apiClient.createCommunity(this.newCommunity).then((resp: ApiData<SquadOvCommunity>) => {
            if (!!this.myCommunities) {
                this.myCommunities.push(resp.data)
            }
            this.showHideCreate = false
            this.newCommunity = createDefaultCommunity()
        }).catch((err: any) => {
            console.error('Failed to create community: ', err)
            if (!!err.response?.data.duplicateFlag) {
                this.createDupError = true
            } else {
                this.createError = true
            }
        }).finally(() => {
            this.createPending = false
        })
    }

    mounted() {
        this.refreshCommunities()
    }

    get nameRules(): any[] {
        return [
            (value : any) => (!!value && value.trim().length > 0) || 'Required.',
        ]
    }

    get slugRules(): any[] {
        return [
            (value : any) => (!!value && value.trim().length > 0) || 'Required.',
        ]
    }
}

</script>