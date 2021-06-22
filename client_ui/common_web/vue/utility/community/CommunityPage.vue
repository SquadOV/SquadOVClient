<template>
    <div>
        <loading-container :is-loading="!community || !roles || !sub">
            <template v-slot:default="{ loading }">
                <div v-if="!loading">
                    <div class="d-flex align-center">
                        <community-list-item
                            :community="community"
                            no-link
                        >
                        </community-list-item>

                        <v-spacer></v-spacer>

                        <v-btn icon v-if="canManage">
                            <v-icon>
                                mdi-cog
                            </v-icon>
                        </v-btn>
                    </div>
                    <v-divider class="my-2"></v-divider>
                        
                    <div
                        v-if="!isPublic && roles.length === 0"
                        class="d-flex flex-column justify-center align-center ma-4"
                    >
                        <v-icon x-large color="warning">
                            mdi-alert
                        </v-icon>
                        
                        <div class="d-flex justify-center" style="width: 60%; max-width: 60%;">
                            <div v-if="!isLoggedIn">
                                <div class="font-weight-bold text-h6">
                                    You must login to view/join this community.
                                </div>

                                <div class="d-flex justify-center mt-4">
                                    <v-btn
                                        class="mx-1"
                                        color="success"
                                        :to="registerTo"
                                    >
                                        Register
                                    </v-btn>

                                    <v-btn
                                        class="mx-1"
                                        color="primary"
                                        :to="loginTo"
                                    >
                                        Login
                                    </v-btn>
                                </div>
                            </div>

                            <div class="d-flex align-center justify-center" v-else-if="needsSubToJoin">
                                <template v-if="hasSub">
                                    <div class="font-weight-bold text-h6">
                                        You must join the community to see posts.
                                    </div>

                                    <community-join-button
                                        :community="community"
                                    >
                                    </community-join-button>
                                </template>

                                <template v-else>
                                    <div class="font-weight-bold text-h6">
                                        This community requires you to subscribe to the community creator prior to joining.
                                    </div>

                                    <community-subscription-button
                                        :community="community"
                                        :sub.sync="sub"
                                    >
                                    </community-subscription-button>
                                </template>
                            </div>

                            <div class="font-weight-bold text-h6" v-else>
                                This community is invite-only.
                            </div>
                        </div>
                    </div>

                    <div v-else>
                    </div>
                </div>
            </template>
        </loading-container>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import CommunityListItem from '@client/vue/utility/community/CommunityListItem.vue'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { SquadOvCommunity, CommunityRole, CommunitySecurityLevel } from '@client/js/squadov/community'
import { User2UserSubscription } from '@client/js/squadov/subscription'
import * as pi from '@client/js/pages'
import CommunityJoinButton from '@client/vue/utility/community/CommunityJoinButton.vue'
import CommunitySubscriptionButton from '@client/vue/utility/community/CommunitySubscriptionButton.vue'

@Component({
    components: {
        LoadingContainer,
        CommunityListItem,
        CommunityJoinButton,
        CommunitySubscriptionButton,   
    }
})
export default class CommunityPage extends Vue {
    @Prop({required: true})
    slug!: string

    community: SquadOvCommunity | null = null
    roles: CommunityRole[] | null = null
    sub: User2UserSubscription[] | null = null

    get isPublic(): boolean {
        if (!this.community) {
            return false
        }
        return this.community.securityLevel === CommunitySecurityLevel.Public
    }

    get isLoggedIn(): boolean {
        return !!this.$store.state.currentUser
    }

    get canManage(): boolean {
        return this.isLoggedIn && !!this.roles && this.roles.some((ele: CommunityRole) => ele.canManage)
    }
    
    get needsSubToJoin(): boolean {
        if (!this.community) {
            return false
        }
        return this.community.securityLevel === CommunitySecurityLevel.Private && this.community.requiresSubscription
    }

    get hasSub(): boolean {
        if (!this.community || !this.sub) {
            return false
        }

        if (this.community.allowTwitchSub) {
            return this.sub.length > 0
        } else {
            return this.sub.some((ele: User2UserSubscription) => !ele.isTwitch)
        }
    }

    @Watch('slug')
    @Watch('isLoggedIn')
    refreshData() {
        this.community = null
        apiClient.getCommunityFromSlug(this.slug).then((resp: ApiData<SquadOvCommunity>) => {
            this.community = resp.data
        }).catch((err: any) => {
            console.log('Failed to get community from slug: ', err)
        })

        this.roles = null
        this.sub = null
        if (this.isLoggedIn) {
            apiClient.getMyCommunityRolesFromSlug(this.slug).then((resp: ApiData<CommunityRole[]>) => {
                this.roles = [] // resp.data
            }).catch((err: any) => {
                console.log('Failed to get community roles: ', err)
                this.roles = []
            })

            apiClient.getMyCommunitySubscription(this.slug).then((resp: ApiData<User2UserSubscription[]>) => {
                this.sub = resp.data
            }).catch((err: any) => {
                console.log('Failed to get community sub: ', err)
            })
        } else {
            this.roles = []
            this.sub = []
        }
    }

    mounted() {
        this.refreshData()
    }

    get registerTo(): any {
        return {
            name: pi.RegisterPageId
        }
    }

    get loginTo(): any {
        return {
            name: pi.LoginPageId
        }
    }
}

</script>