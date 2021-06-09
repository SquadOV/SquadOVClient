<template>
    <v-tabs vertical v-model="internalTab">
        <v-tab>
            Basic
        </v-tab>

        <v-tab-item>
            <loading-container :is-loading="!profileUser">
                <template v-slot:default="{ loading }">
                    <div v-if="!loading">
                        <v-container fluid>
                            <v-row class="justify-center">
                                <v-col cols="8">
                                    <span class="text-h5 text--white font-weight-bold">
                                        User Profile
                                    </span>
                                </v-col>
                            </v-row>

                            <v-divider class="my-4"></v-divider>

                            <v-row class="justify-center">
                                <v-col cols="4">
                                    <div class="font-weight-bold">
                                        Username
                                    </div>
                                    
                                    <div>
                                        {{ profileUser.username }}
                                    </div>
                                </v-col>

                                <v-col cols="4" v-if="isLocal">
                                    <div class="font-weight-bold">
                                        Email
                                    </div>

                                    <div class="d-flex align-center">
                                        {{ profileUser.email }}
                                        <v-icon
                                            class="ml-2"
                                            color="success"
                                            small
                                            v-if="profileUser.verified"
                                        >
                                            mdi-check-circle
                                        </v-icon>
                                        
                                        <template v-else>
                                            <v-icon
                                                class="ml-2"
                                                color="error"
                                                small
                                            >
                                                mdi-close-circle
                                            </v-icon>

                                            <v-btn
                                                class="ml-2"
                                                color="success"
                                                small
                                                v-if="!sentVerification"
                                                @click="resendVerification"
                                                :loading="emailPending" 
                                            >
                                                Resend Email Verification
                                            </v-btn>

                                            <div class="ml-2" v-else>
                                                Email Sent!
                                            </div>

                                            <v-btn
                                                class="ml-2"
                                                color="secondary"
                                                small
                                                @click="recheckVerification"
                                                :loading="checkPending"
                                            >
                                                Refresh
                                            </v-btn>
                                        </template>
                                    </div>
                                </v-col>
                            </v-row>
                        </v-container>
                    </div>
                </template>
            </loading-container>
        </v-tab-item>

        <v-tab>
            Matches
        </v-tab>

        <v-tab-item>
            <v-tabs v-model="internalMatchTab">
                <v-tab>
                    Favorites
                </v-tab>

                <v-tab-item>
                    <user-match-favorites
                        :user-id="userId"
                    >
                    </user-match-favorites>
                </v-tab-item>

                <v-tab>
                    Watchlist
                </v-tab>

                <v-tab-item>
                    <user-match-watchlist
                        :user-id="userId"
                    >
                    </user-match-watchlist>
                </v-tab-item>
            </v-tabs>
        </v-tab-item>

        <v-tab>
            Clips
        </v-tab>

        <v-tab-item>
            <v-tabs>
                <v-tab>
                    Favorites
                </v-tab>

                <v-tab-item>
                    <user-clip-favorites
                        :user-id="userId"
                    >
                    </user-clip-favorites>
                </v-tab-item>

                <v-tab>
                    Watchlist
                </v-tab>

                <v-tab-item>
                    <user-clip-watchlist
                        :user-id="userId"
                    >
                    </user-clip-watchlist>
                </v-tab-item>
            </v-tabs>
        </v-tab-item>
    </v-tabs>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { SquadOVUser, getSquadOVUser } from '@client/js/squadov/user'
import { apiClient, ApiData } from '@client/js/api' 
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import UserMatchFavorites from '@client/vue/profile/UserMatchFavorites.vue'
import UserMatchWatchlist from '@client/vue/profile/UserMatchWatchlist.vue'
import UserClipFavorites from '@client/vue/profile/UserClipFavorites.vue'
import UserClipWatchlist from '@client/vue/profile/UserClipWatchlist.vue'

@Component({
    components: {
        LoadingContainer,
        UserMatchFavorites,
        UserMatchWatchlist,
        UserClipFavorites,
        UserClipWatchlist,
    }
})
export default class UserProfile extends Vue {
    @Prop({required: true})
    userId!: number

    @Prop({default: 0})
    tab!: number

    @Prop({default: 0})
    matchTab!: number

    profileUser: SquadOVUser | null = null

    internalTab: number = 0
    internalMatchTab: number = 0

    sentVerification: boolean = false
    emailPending: boolean = false
    verificationErr: boolean = false
    checkPending: boolean = false

    recheckVerification() {
        this.checkPending = true
        getSquadOVUser(this.$store.state.currentUser.id).then((resp: ApiData<SquadOVUser>) => {
            this.$store.commit('setUser' , resp.data)
            this.profileUser = resp.data
        }).catch((err: any) => {
            console.log('Failed to refresh squadovuser : ', err)
        }).finally(() => {
            this.checkPending = false
        })
    }

    resendVerification() {
        this.emailPending = true
        apiClient.resendVerification().then(() => {
            this.sentVerification = true
        }).catch((err: any) => {
            console.log('Failed to resend verification: ', err)
            this.verificationErr = false
        }).finally(() => {
            this.emailPending = false
        })
    }

    get localUserId(): number {
        return this.$store.state.currentUser!.id
    }

    get isLocal(): boolean {
        return this.localUserId == this.profileUser?.id
    }

    @Watch('userId')
    refreshData() {
        this.profileUser = null
        getSquadOVUser(this.userId).then((resp: ApiData<SquadOVUser>) => {
            this.profileUser = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain user profile: ', err)
        })
    }

    mounted() {
        this.internalTab = this.tab
        this.internalMatchTab = this.matchTab

        this.refreshData()
    }
}

</script>