<template>
    <loading-container :is-loading="!userProfile">
        <template v-slot:default="{ loading }">
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
                    <div v-html="userProfile.misc.description"></div>
                </div>
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
                            <v-container fluid>
                                <v-row justify="center">
                                    <v-col class="d-flex" align-self="center" cols="4">
                                        Profile Picture
                                    </v-col>

                                    <v-col cols="8">
                                        <v-text-field
                                            outlined
                                            dense
                                            hide-details
                                        >
                                        </v-text-field>
                                    </v-col>
                                </v-row>

                                <v-row justify="center">
                                    <v-col class="d-flex" align-self="center" cols="4">
                                        Cover Photo
                                    </v-col>

                                    <v-col cols="8">
                                        <v-text-field
                                            outlined
                                            dense
                                            hide-details
                                        >
                                        </v-text-field>
                                    </v-col>
                                </v-row>

                                <v-row justify="center">
                                    <v-col class="d-flex" align-self="center" cols="4">
                                        Display Name
                                    </v-col>

                                    <v-col cols="8">
                                        <v-text-field
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
                                            outlined
                                            dense
                                            hide-details
                                        >
                                        </v-text-field>
                                    </v-col>
                                </v-row>
                            </v-container>
                        </div>

                        <v-card-actions>
                            <v-btn
                                color="error"
                                @click="showHideEditProfile = false"
                            >
                                Cancel
                            </v-btn>

                            <v-spacer></v-spacer>

                            <v-btn
                                color="success"
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
                            <v-container fluid>
                                <v-row justify="center">
                                    <v-col class="d-flex" align-self="center" cols="4">
                                        <div>
                                            <div>URL</div>
                                            <div class="text-caption">
                                                https://app.squadov.gg/p/{{ editableProfile.linkSlug }}
                                            </div>
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

                                <v-row justify="center">
                                    <v-col class="d-flex" align-self="center" cols="4">
                                        Access
                                    </v-col>

                                    <v-col cols="8">
                                        <user-profile-access-editor
                                            v-model="editableProfile.matchAccess"
                                            label="Games"
                                            tooltip="Who can see your profile's VODs and clips."
                                        >
                                        </user-profile-access-editor>

                                        <user-profile-access-editor
                                            class="mt-4"
                                            v-model="editableProfile.achievementAccess"
                                            label="Ranks"
                                            tooltip="Who can see your profile's ranks and gaming achievements."
                                        >
                                        </user-profile-access-editor>

                                        <user-profile-access-editor
                                            class="mt-4"
                                            v-model="editableProfile.miscAccess"
                                            label="Misc."
                                            tooltip="Who can see your profile's description and social links."
                                        >
                                        </user-profile-access-editor>
                                    </v-col>
                                </v-row>
                            </v-container>
                        </div>

                        <v-card-actions>
                            <v-btn
                                color="error"
                                @click="showHideEditAccess = false"
                            >
                                Cancel
                            </v-btn>

                            <v-spacer></v-spacer>

                            <v-btn
                                color="success"
                            >
                                Save
                            </v-btn>
                        </v-card-actions>
                    </v-card>
                </v-dialog>
                
            </div>
            <v-divider class="my-4"></v-divider>

            <!-- Profile Data -->
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { UserProfileBasic, cleanUserProfileBasicFromJson } from '@client/js/squadov/user'
import { apiClient, ApiData } from '@client/js/api'

import FacebookLink from '@client/vue/utility/social/FacebookLink.vue'
import InstagramLink from '@client/vue/utility/social/InstagramLink.vue'
import TwitchLink from '@client/vue/utility/social/TwitchLink.vue'
import YoutubeLink from '@client/vue/utility/social/YoutubeLink.vue'
import SnapchatLink from '@client/vue/utility/social/SnapchatLink.vue'
import TwitterLink from '@client/vue/utility/social/TwitterLink.vue'
import TiktokLink from '@client/vue/utility/social/TiktokLink.vue'
import UserProfileAccessEditor from '@client/vue/profile/UserProfileAccessEditor.vue'

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
    }
})
export default class UserProfile extends Vue {
    // One of userId or profileSlug must be set so we know
    // which user's profile to access. 
    @Prop()
    userId!: number | undefined

    @Prop()
    profileSlug!: string | undefined

    userProfile: UserProfileBasic | null = null
    showHideEditProfile: boolean = false
    showHideEditAccess: boolean = false

    editableProfile: UserProfileBasic | null = null

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
        return this.$store.state.currentUser.id === this.userProfile?.userId
    }

    @Watch('userId')
    @Watch('profileSlug')
    refreshProfile() {
        let promise
        this.userProfile = null
        if (this.userId !== undefined) {
            promise = apiClient.getBasicUserProfileFromId(this.userId)
        } else if (this.profileSlug !== undefined) {
            promise = apiClient.getBasicUserProfileFromSlug(this.profileSlug)
        } else {
            console.log('No user ID or profile slug found to get user profile.')
            return
        }

        promise.then((resp: ApiData<UserProfileBasic>) => {
            this.userProfile = resp.data
        }).catch((err: any) => {
            console.log('Failed to get user profile basic: ', err)
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

    mounted() {
        this.refreshProfile()
    }
}

</script>

<style scoped>

#misc-div {
    max-width: 60vw;
}

#profile-image {
    position: absolute;
    left: 50%;
    transform: translateX(-50%) translateY(-100%) translateY(16px);
    background-color: #303030;
    border: 3px solid white;
    border-radius: 50%;
}

.edit-content {
    overflow: auto;
    max-height: 80vh;
}

</style>