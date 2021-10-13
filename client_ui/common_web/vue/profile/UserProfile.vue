<template>
    <loading-container :is-loading="userProfileLoading">
        <template v-slot:default="{ loading }">
            <template v-if="!loading">
                <template v-if="!!userProfile">
                    <!-- Cover photo and user profile picture -->
                    <div class="full-width mb-8">
                        <v-img
                            :src="coverPhotoUrl"
                            height="25vh"
                            max-height="400px"
                            width="100%"
                            max-width="100%"
                        >
                        </v-img>

                        <v-img
                            id="profile-image"
                            class="profile-image"
                            :src="profileImageUrl"
                            height="128px"
                            width="128px"
                        >
                        </v-img>
                    </div>

                    <!-- Username/Display Name -->
                    <div class="d-flex flex-column align-center justify-center">
                        <div class="text-h4 font-weight-bold">
                            {{ userProfile.displayName }}
                        </div>

                        <div class="text-subtitle-2" v-if="userProfile.displayName != userProfile.username">
                            A.K.A&nbsp;{{ userProfile.username }}
                        </div>
                    </div>

                    <div class="d-flex justify-center" v-if="!!userProfile.misc">
                        <div id="misc-div">
                            <!-- Socials -->
                            <div class="mt-1 d-flex align-center justify-center flex-wrap">
                                <facebook-link
                                    v-if="!!userProfile.misc.links.facebook"
                                    :input="userProfile.misc.links.facebook"
                                >
                                </facebook-link>

                                <instagram-link
                                    class="ml-3"
                                    v-if="!!userProfile.misc.links.instagram"
                                    :input="userProfile.misc.links.instagram"
                                >
                                </instagram-link>

                                <twitch-link
                                    class="ml-3"
                                    v-if="!!userProfile.misc.links.twitch"
                                    :input="userProfile.misc.links.twitch"
                                >
                                </twitch-link>

                                <youtube-link
                                    class="ml-3"
                                    v-if="!!userProfile.misc.links.youtube"
                                    :input="userProfile.misc.links.youtube"
                                >
                                </youtube-link>

                                <snapchat-link
                                    class="ml-3"
                                    v-if="!!userProfile.misc.links.snapchat"
                                    :input="userProfile.misc.links.snapchat"
                                >
                                </snapchat-link>

                                <twitter-link
                                    class="ml-3"
                                    v-if="!!userProfile.misc.links.twitter"
                                    :input="userProfile.misc.links.twitter"
                                >
                                </twitter-link>

                                <tiktok-link
                                    class="ml-3"
                                    v-if="!!userProfile.misc.links.tiktok"
                                    :input="userProfile.misc.links.tiktok"
                                >
                                </tiktok-link>
                            </div>

                            <!-- Description -->
                            <div class="mt-1 d-flex align-center justify-center flex-wrap text-subtitle-2">
                                {{ userProfile.misc.description }}
                            </div>
                        </div>
                    </div>

                    <!-- Member Since -->
                    <div class="mt-1 d-flex align-center justify-center flex-wrap text-caption">
                        <span class="font-weight-bold">Member Since:&nbsp;</span> {{ standardFormatTime(userProfile.memberSince) }}
                    </div>

                    <!-- Edit Buttons -->
                    <div class="d-flex justify-center align-center mt-4" v-if="canEdit">
                        <v-dialog
                            v-model="showHideEditProfile"
                            persistent
                            max-width="40%"
                        >
                            <template v-slot:activator="{on, attrs}">
                                <v-btn
                                    v-bind="attrs"
                                    v-on="on"
                                    color="primary"
                                >
                                    Edit Profile
                                </v-btn>
                            </template>

                            <v-card>
                                <v-card-title>
                                    Edit Your Profile
                                </v-card-title>
                                <v-divider></v-divider>

                                <div class="ma-4 edit-content">
                                    <v-form v-model="profileValid">
                                        <v-container fluid>
                                            <v-row justify="center">
                                                <v-col class="d-flex" align-self="center" cols="4">
                                                    Profile Picture
                                                </v-col>
                                                <v-col cols="8">
                                                    <v-file-input
                                                        v-model="stagedProfilePhoto"
                                                        outlined
                                                        dense
                                                        accept="image/*"
                                                        show-size
                                                        :rules="profilePicRules"
                                                    >
                                                    </v-file-input>
                                                </v-col>
                                            </v-row>
                                            <v-row justify="center">
                                                <v-col cols="12">
                                                    <div class="d-flex justify-center">
                                                        <v-img
                                                            class="profile-image"
                                                            :src="stagedProfileImageUrl"
                                                            height="128px"
                                                            max-height="128px"
                                                            width="128px"
                                                            max-width="128px"
                                                        >
                                                        </v-img>
                                                    </div>
                                                </v-col>
                                            </v-row>
                                            <v-row justify="center">
                                                <v-col cols="12">
                                                    <div class="d-flex justify-center">
                                                        <user-ready-player-me 
                                                            @input="stagedProfilePhoto = arguments[0]"
                                                        >
                                                        </user-ready-player-me>
                                                    </div>
                                                </v-col>
                                            </v-row>
                                            <v-row justify="center">
                                                <v-col class="d-flex" align-self="center" cols="4">
                                                    Cover Photo
                                                </v-col>

                                                <v-col cols="8">
                                                    <v-file-input
                                                        v-model="stagedCoverPhoto"
                                                        outlined
                                                        dense
                                                        accept="image/*"
                                                        show-size
                                                        :rules="coverPhotoRules"
                                                    >
                                                    </v-file-input>
                                                </v-col>
                                            </v-row>

                                            <v-row justify="center">
                                                <v-col cols="12">
                                                    <div class="d-flex justify-center">
                                                        <v-img
                                                            :src="stagedCoverPhotoUrl"
                                                        >
                                                        </v-img>
                                                    </div>
                                                </v-col>
                                            </v-row>

                                            <v-row justify="center">
                                                <v-col class="d-flex" align-self="center" cols="4">
                                                    Display Name
                                                </v-col>

                                                <v-col cols="8">
                                                    <v-text-field
                                                        v-model="editableProfile.displayName"
                                                        outlined
                                                        dense
                                                        hide-details
                                                    >
                                                    </v-text-field>
                                                </v-col>
                                            </v-row>

                                            <v-row justify="center">
                                                <v-col class="d-flex" align-self="center" cols="4">
                                                    Description
                                                </v-col>

                                                <v-col cols="8">
                                                    <v-text-field
                                                        v-model="editableProfile.misc.description"
                                                        outlined
                                                        dense
                                                        hide-details
                                                    >
                                                    </v-text-field>
                                                </v-col>
                                            </v-row>

                                            <v-row justify="center">
                                                <v-col class="d-flex" align-self="center" cols="4">
                                                    Social Media
                                                </v-col>

                                                <v-col cols="8">
                                                    <v-text-field
                                                        v-model="editableProfile.misc.links.facebook"
                                                        outlined
                                                        dense
                                                        hide-details
                                                    >
                                                        <template v-slot:prepend-inner>
                                                            <v-img
                                                                :src="$root.generateAssetUri('assets/profile/social/facebook.svg')"
                                                                width="16px"
                                                                height="16px"
                                                                contain
                                                            >
                                                            </v-img>
                                                        </template>
                                                    </v-text-field>

                                                    <v-text-field
                                                        class="mt-2"
                                                        v-model="editableProfile.misc.links.instagram"
                                                        outlined
                                                        dense
                                                        hide-details
                                                    >
                                                        <template v-slot:prepend-inner>
                                                            <v-img
                                                                :src="$root.generateAssetUri('assets/profile/social/instagram.svg')"
                                                                width="16px"
                                                                height="16px"
                                                                contain
                                                            >
                                                            </v-img>
                                                        </template>
                                                    </v-text-field>

                                                    <v-text-field
                                                        class="mt-2"
                                                        v-model="editableProfile.misc.links.snapchat"
                                                        outlined
                                                        dense
                                                        hide-details
                                                    >
                                                        <template v-slot:prepend-inner>
                                                            <v-img
                                                                :src="$root.generateAssetUri('assets/profile/social/snapchat.png')"
                                                                width="16px"
                                                                height="16px"
                                                                contain
                                                            >
                                                            </v-img>
                                                        </template>
                                                    </v-text-field>

                                                    <v-text-field
                                                        class="mt-2"
                                                        v-model="editableProfile.misc.links.tiktok"
                                                        outlined
                                                        dense
                                                        hide-details
                                                    >
                                                        <template v-slot:prepend-inner>
                                                            <v-img
                                                                :src="$root.generateAssetUri('assets/profile/social/tiktok.png')"
                                                                width="16px"
                                                                height="16px"
                                                                contain
                                                            >
                                                            </v-img>
                                                        </template>
                                                    </v-text-field>

                                                    <v-text-field
                                                        class="mt-2"
                                                        v-model="editableProfile.misc.links.twitch"
                                                        outlined
                                                        dense
                                                        hide-details
                                                    >
                                                        <template v-slot:prepend-inner>
                                                            <v-img
                                                                :src="$root.generateAssetUri('assets/profile/social/twitch.png')"
                                                                width="16px"
                                                                height="16px"
                                                                contain
                                                            >
                                                            </v-img>
                                                        </template>
                                                    </v-text-field>

                                                    <v-text-field
                                                        class="mt-2"
                                                        v-model="editableProfile.misc.links.twitter"
                                                        outlined
                                                        dense
                                                        hide-details
                                                    >
                                                        <template v-slot:prepend-inner>
                                                            <v-img
                                                                :src="$root.generateAssetUri('assets/profile/social/twitter.svg')"
                                                                width="16px"
                                                                height="16px"
                                                                contain
                                                            >
                                                            </v-img>
                                                        </template>
                                                    </v-text-field>

                                                    <v-text-field
                                                        class="mt-2"
                                                        v-model="editableProfile.misc.links.youtube"
                                                        outlined
                                                        dense
                                                        hide-details
                                                    >
                                                        <template v-slot:prepend-inner>
                                                            <v-img
                                                                :src="$root.generateAssetUri('assets/profile/social/youtube.svg')"
                                                                width="16px"
                                                                height="16px"
                                                                contain
                                                            >
                                                            </v-img>
                                                        </template>
                                                    </v-text-field>
                                                </v-col>
                                            </v-row>
                                        </v-container>
                                    </v-form>
                                </div>

                                <v-card-actions>
                                    <v-btn
                                        color="error"
                                        @click="cancelEdit"
                                    >
                                        Cancel
                                    </v-btn>

                                    <v-spacer></v-spacer>

                                    <v-btn
                                        color="success"
                                        :disabled="!profileValid"
                                        :loading="profileEditInProgress"
                                        @click="saveProfile"
                                    >
                                        Save
                                    </v-btn>
                                </v-card-actions>
                            </v-card>
                        </v-dialog>

                        <v-dialog
                            v-model="showHideEditAccess"
                            persistent
                            max-width="40%"
                        >
                            <template v-slot:activator="{on, attrs}">
                                <v-btn
                                    class="ml-2"
                                    v-bind="attrs"
                                    v-on="on"
                                >
                                    Edit Sharing
                                </v-btn>
                            </template>

                            <v-card>
                                <v-card-title>
                                    Edit Sharing
                                </v-card-title>
                                <v-divider></v-divider>

                                <div class="ma-4 edit-content">
                                    <v-form v-model="accessValid">
                                        <v-container fluid>
                                            <v-row justify="center">
                                                <v-col class="d-flex" align-self="center" cols="4">
                                                    <div>
                                                        <div>URL</div>
                                                    </div>
                                                </v-col>

                                                <v-col cols="8">
                                                    <v-text-field
                                                        v-model="editableProfile.linkSlug"
                                                        :rules="urlRules"
                                                        outlined
                                                        dense
                                                    >
                                                    </v-text-field>
                                                </v-col>
                                            </v-row>

                                            <v-row>
                                                <div class="text-caption">
                                                    https://app.squadov.gg/p/{{ editableProfile.linkSlug }}
                                                </div>
                                            </v-row>

                                            <v-row justify="center">
                                                <v-col class="d-flex" align-self="center" cols="4">
                                                    Access
                                                </v-col>

                                                <v-col cols="8">
                                                    <user-profile-access-editor
                                                        v-model="editableProfile.matchAccess"
                                                        :twitch="!!userProfile.twitchChannelForSub"
                                                        label="Games and Clips"
                                                        tooltip="Who can see your profile's VODs and clips."
                                                    >
                                                    </user-profile-access-editor>

                                                    <!--
                                                    <user-profile-access-editor
                                                        class="mt-4"
                                                        v-model="editableProfile.achievementAccess"
                                                        :twitch="!!userProfile.twitchChannelForSub"
                                                        label="Ranks"
                                                        tooltip="Who can see your profile's ranks and gaming achievements."
                                                    >
                                                    </user-profile-access-editor>
                                                    -->

                                                    <user-profile-access-editor
                                                        class="mt-4"
                                                        v-model="editableProfile.miscAccess"
                                                        :twitch="!!userProfile.twitchChannelForSub"
                                                        label="Misc."
                                                        tooltip="Who can see your profile's description and social links."
                                                    >
                                                    </user-profile-access-editor>
                                                </v-col>
                                            </v-row>
                                        </v-container>
                                    </v-form>
                                </div>

                                <v-card-actions>
                                    <v-btn
                                        color="error"
                                        @click="cancelEdit"
                                    >
                                        Cancel
                                    </v-btn>

                                    <v-spacer></v-spacer>

                                    <v-btn
                                        color="success"
                                        :disabled="!accessValid"
                                        :loading="profileEditInProgress"
                                        @click="saveAccess"
                                    >
                                        Save
                                    </v-btn>
                                </v-card-actions>
                            </v-card>
                        </v-dialog>  
                    </div>

                    <div class="mt-4" v-else-if="needTwitchSub">
                        <div class="d-flex justify-center align-center text-subtitle-2">
                            This user has chosen to make parts of their profile private, subscribe to them on Twitch for access!
                        </div>

                        <div class="d-flex justify-center align-center mt-2">
                            <template v-if="!!$store.state.currentUser">
                                <v-btn
                                    @click="redirectToTwitch"
                                    color="#9147ff"
                                >
                                    Subscribe on Twitch
                                </v-btn>

                                <v-btn
                                    @click="refreshProfile"
                                    class="ml-2"
                                >
                                    Refresh
                                </v-btn>
                            </template>

                            <template v-else>
                                <v-btn
                                    color="success"
                                    :to="registerTo"
                                >
                                    Register
                                </v-btn>

                                <v-btn
                                    class="ml-2"
                                    color="primary"
                                    :to="loginTo"
                                >
                                    Login
                                </v-btn>
                            </template>
                        </div>
                    </div>

                    <v-divider class="mt-2"></v-divider>

                    <!-- Profile Data -->
                    <v-container fluid>
                        <v-row justify="center" v-if="userProfile.hasMatchAccess">
                            <v-col cols="4">
                            <!--
                                <user-profile-achievements
                                    :user-id="userProfile.userId"
                                    v-if="userProfile.hasAchievementAccess"
                                >
                                </user-profile-achievements>
                            -->

                                <user-profile-clips
                                    :user-id="userProfile.userId"
                                    :access-token="userProfile.accessToken"
                                    class="mb-4"
                                >
                                </user-profile-clips>
                            </v-col>

                            <v-col cols="8">
                                <user-profile-matches
                                    :user-id="userProfile.userId"
                                    :access-token="userProfile.accessToken"
                                >
                                </user-profile-matches>
                            </v-col>
                        </v-row>

                        <v-row justify="center" v-else-if="!!$store.state.currentUser">
                            <v-col cols="6">
                                <div class="d-flex justify-center">
                                    {{ userProfile.displayName }} has chosen to keep their profile private.
                                    You must be a part of their squad to view their profile.
                                </div>
                            </v-col>
                        </v-row>

                        <v-row justify="center" v-else>
                            <v-col cols="6">
                                <div class="d-flex justify-center">
                                    {{ userProfile.displayName }} has chosen to keep their profile private.
                                </div>

                                <div class="d-flex justify-center align-center mt-2">
                                    <v-btn
                                        color="success"
                                        :to="registerTo"
                                    >
                                        Register
                                    </v-btn>

                                    <v-btn
                                        class="ml-2"
                                        color="primary"
                                        :to="loginTo"
                                    >
                                        Login
                                    </v-btn>
                                </div>
                            </v-col>
                        </v-row>
                    </v-container>

                    <v-snackbar
                        v-model="editFailure"
                        :timeout="5000"
                        color="error"
                    >
                        Failed to edit your profile. Please try again.
                    </v-snackbar>
                </template>

                <div class="d-flex flex-column justify-center align-center full-width full-parent-height" v-else>
                    <template v-if="canCreateProfile">
                        <span class="text-h6">Welcome to your user profile! Choose your preferred URL to get started.</span>
                        
                        <v-form class="mt-1" v-model="createValid">
                            <v-text-field
                                v-model="createSlug"
                                :rules="urlRules"
                                outlined
                                dense
                            >
                            </v-text-field>
                        </v-form>

                        <span class="text-subtitle-2">https://app.squadov.gg/p/{{ createSlug }}</span>

                        <v-btn
                            class="mt-4"
                            :disabled="!createValid"
                            :loading="createInProgress"
                            color="primary"
                            @click="createProfile"
                        >
                            Create My Profile
                        </v-btn>
                    </template>
                    <span class="text-h6" v-else>We're in the process of rolling out access to user profiles, check back soon!</span>
                </div>

                <v-snackbar
                    v-model="createFailure"
                    :timeout="5000"
                    color="error"
                >
                    Failed to create your profile. Please try again.
                </v-snackbar>
            </template>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { UserProfileBasic, cleanUserProfileBasicFromJson, USER_PROFILE_ACCESS_PRIVATE_TWITCH_SUB } from '@client/js/squadov/user'
import { apiClient, ApiData } from '@client/js/api'
import { standardFormatTime } from '@client/js/time'
import { openUrlInBrowser} from '@client/js/external'
import * as pi from '@client/js/pages'

import FacebookLink from '@client/vue/utility/social/FacebookLink.vue'
import InstagramLink from '@client/vue/utility/social/InstagramLink.vue'
import TwitchLink from '@client/vue/utility/social/TwitchLink.vue'
import YoutubeLink from '@client/vue/utility/social/YoutubeLink.vue'
import SnapchatLink from '@client/vue/utility/social/SnapchatLink.vue'
import TwitterLink from '@client/vue/utility/social/TwitterLink.vue'
import TiktokLink from '@client/vue/utility/social/TiktokLink.vue'
import UserProfileAccessEditor from '@client/vue/profile/UserProfileAccessEditor.vue'
import UserProfileAchievements from '@client/vue/profile/UserProfileAchievements.vue'
import UserProfileMatches from '@client/vue/profile/UserProfileMatches.vue'
import UserProfileClips from '@client/vue/profile/UserProfileClips.vue'
import UserReadyPlayerMe from '@client/vue/profile/UserReadyPlayerMe.vue'
import axios from 'axios'

@Component({
    components: {
        LoadingContainer,
        FacebookLink,
        InstagramLink,
        TwitchLink,
        YoutubeLink,
        SnapchatLink,
        TwitterLink,
        TiktokLink,
        UserProfileAccessEditor,
        UserProfileAchievements,
        UserProfileMatches,
        UserProfileClips,
        UserReadyPlayerMe,
    }
})
export default class UserProfile extends Vue {
    standardFormatTime = standardFormatTime

    // One of userId or profileSlug must be set so we know
    // which user's profile to access. 
    @Prop()
    userId!: number | undefined

    @Prop()
    profileSlug!: string | undefined

    userProfile: UserProfileBasic | null = null
    userProfileLoading: boolean = false

    showHideEditProfile: boolean = false
    profileValid: boolean = false
    
    showHideEditAccess: boolean = false
    accessValid: boolean = false
    createValid: boolean = false

    profileEditInProgress: boolean = false
    editableProfile: UserProfileBasic | null = null
    stagedProfilePhoto: File | null = null
    stagedCoverPhoto: File | null = null

    createSlug: string = ''
    createInProgress: boolean = false

    editFailure: boolean = false
    createFailure: boolean = false

    redirectToTwitch() {
        if (!this.userProfile?.twitchChannelForSub) {
            return
        }
        openUrlInBrowser(`https://www.twitch.tv/${this.userProfile.twitchChannelForSub}`)
    }

    createProfile() {
        this.createInProgress = true
        apiClient.createUserProfile(this.createSlug).then((resp: ApiData<UserProfileBasic>) => {
            this.userProfile = resp.data
        }).catch((err: any) => {
            console.error('Failed to create user profile: ', err)
            this.createFailure = true
        }).finally(() => {
            this.createInProgress = false
        })
    }

    get canCreateProfile(): boolean {
        return this.$store.state.features.enableUserProfiles && !this.profileSlug && !!this.userId
            && this.userId === this.$store.state.currentUser.id
    }

    get needTwitchSub(): boolean {
        if (!this.userProfile) {
            return false
        }

        if (!this.userProfile.twitchChannelForSub) {
            return false
        }

        if (!this.userProfile.misc && this.userProfile.miscAccess & USER_PROFILE_ACCESS_PRIVATE_TWITCH_SUB) {
            return true
        }

/*
        if (!this.userProfile.hasAchievementAccess && this.userProfile.achievementAccess & USER_PROFILE_ACCESS_PRIVATE_TWITCH_SUB) {
            return true
        }
*/

        if (!this.userProfile.hasMatchAccess && this.userProfile.matchAccess & USER_PROFILE_ACCESS_PRIVATE_TWITCH_SUB) {
            return true
        }

        return false
    }

    get stagedCoverPhotoUrl(): string {
        if (!!this.stagedCoverPhoto) {
            return URL.createObjectURL(this.stagedCoverPhoto)
        } else {
            return this.coverPhotoUrl
        }
    }

    get stagedProfileImageUrl(): string {
        if (!!this.stagedProfilePhoto) {
            return URL.createObjectURL(this.stagedProfilePhoto)
        } else {
            return this.profileImageUrl
        }
    }

    get coverPhotoUrl(): string {
        if (!!this.userProfile?.misc?.coverPictureUrl) {
            return this.userProfile?.misc?.coverPictureUrl
        } else {
            //@ts-ignore
            return this.$root.generateAssetUri('assets/profile/default_cover_compress.jpg')
        }
    }

    get profileImageUrl(): string {
        if (!!this.userProfile?.misc?.profilePictureUrl) {
            return this.userProfile?.misc?.profilePictureUrl
        } else {
            //@ts-ignore
            return this.$root.generateAssetUri('assets/profile/default_profile.png')
        }
    }

    get canEdit(): boolean {
        if (!this.$store.state.currentUser) {
            return false
        }
        return this.$store.state.currentUser.id === this.userProfile?.userId
    }

    @Watch('userId')
    @Watch('profileSlug')
    refreshProfile() {
        let promise
        this.userProfile = null
        this.userProfileLoading = true
        if (this.userId !== undefined) {
            promise = apiClient.getBasicUserProfileFromId(this.userId)
        } else if (this.profileSlug !== undefined) {
            promise = apiClient.getBasicUserProfileFromSlug(this.profileSlug)
        } else {
            console.error('No user ID or profile slug found to get user profile.')
            return
        }

        promise.then((resp: ApiData<UserProfileBasic | null>) => {
            this.userProfile = resp.data
            console.log('get user profile: ', this.userProfile)
        }).catch((err: any) => {
            console.error('Failed to get user profile basic: ', err)
        }).finally(() => {
            this.userProfileLoading = false
        })
    }

    @Watch('userProfile')
    syncToEditableProfile() {
        if (!this.userProfile) {
            this.editableProfile = null
            return
        }
        this.editableProfile = JSON.parse(JSON.stringify(this.userProfile))
        cleanUserProfileBasicFromJson(this.editableProfile!)
    }

    get urlRules(): any[] {
        const illegalChars = '<>[]{}|\\^%&$+,/:;=?@# '
        return [
            (value : any) => (!!value && value.trim().length > 0) || 'Required.',
            (value : any) => !illegalChars.split('').some((c: string) => (value || '').includes(c)) || `URL may not contain special characters: ${illegalChars} or spaces.`,
            (value : any) => (value || '').length <= 48 || 'URL can have at most 48 characters.',
        ]
    }

    get profilePicRules(): any[] {
        return [
            (value : File | null | undefined) => (!value || (!!value && value.size <= 1 * 1024 * 1024)) || 'Image can not be larger than 1MB.'
        ]
    }

    get coverPhotoRules(): any[] {
        return [
            (value : File | null | undefined) => (!value || (!!value && value.size <= 1 * 1024 * 1024)) || 'Image can not be larger than 1MB.'
        ]
    }

    cancelEdit() {
        this.syncToEditableProfile()
        this.showHideEditProfile = false
        this.showHideEditAccess = false
        this.stagedCoverPhoto = null
        this.stagedProfilePhoto = null
    }

    saveProfile() {
        if (!this.editableProfile) {
            return
        }
        this.profileEditInProgress = true
        apiClient.updateBasicProfileData(this.editableProfile, this.stagedCoverPhoto, this.stagedProfilePhoto).then(() => {
            this.showHideEditProfile = false
            this.refreshProfile()
        }).catch((err: any) => {
            console.error('Failed to update basic profile data: ', err)
            this.editFailure = true
        }).finally(() => {
            this.profileEditInProgress = false
        })
    }

    saveAccess() {
        if (!this.editableProfile) {
            return
        }
        this.profileEditInProgress = true
        apiClient.updateBasicProfileAccess(this.editableProfile).then(() => {
            this.showHideEditAccess = false
            this.refreshProfile()
        }).catch((err: any) => {
            console.error('Failed to update basic profile access: ', err)
            this.editFailure = true
        }).finally(() => {
            this.profileEditInProgress = false
        })
    }

    mounted() {
        this.refreshProfile()
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

<style scoped>

#misc-div {
    max-width: 60vw;
}

#profile-image {
    position: relative;
    left: 50%;
    transform: translateX(-50%) translateY(-100%) translateY(16px);
    margin-bottom: -128px;
}

.profile-image {
    background-color: #303030;
    border: 3px solid white;
    border-radius: 50%;
}

.edit-content {
    overflow: auto;
    max-height: 70vh;
}

</style>