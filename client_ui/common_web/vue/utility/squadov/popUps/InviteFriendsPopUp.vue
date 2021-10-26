<template>
    <div>
        <v-dialog width="500" v-model="dialog">
            <v-card>
                <v-card-title> Invite your Friends! </v-card-title>
                <v-card-text>
                    It gets lonely here, add your friends! Play, improve, and
                    have fun as a squad!
                </v-card-text>
                <v-card-actions>
                    <v-btn
                        small
                        class="ml-2"
                        color="success"
                        @click="copyToClipboard"
                    >
                        Copy Invite Link!
                    </v-btn>
                    <v-spacer></v-spacer>
                    <v-checkbox
                        dense
                        :label="'Don\'t show again'"
                        v-model="muteFuturePopUp"
                        hide-details
                    ></v-checkbox>
                    <v-spacer></v-spacer>
                    <v-btn @click="closeNotification()" color="error">
                        Close
                    </v-btn>
                </v-card-actions>
            </v-card>
        </v-dialog>
        <v-snackbar v-model="showHideCopy" :timeout="5000" color="success">
            Copied URL to clipboard!
        </v-snackbar>
    </div>
</template>

<script lang="ts">
import Vue from 'vue'
import Component from 'vue-class-component'
import { apiClient, ApiData } from '@client/js/api'
import '@mdi/font/css/materialdesignicons.css'
import { Watch } from 'vue-property-decorator'

@Component
export default class InviteFriendsPopUp extends Vue {
    alertHeight: number = 45
    dialog: boolean = false
    showHideCopy: boolean = false
    refLink: string = ""
    muteFuturePopUp: boolean = false

    get isLoggedIn(): boolean {
        return !!this.$store.state.currentUser
    }

    copyToClipboard() {
        navigator.clipboard.writeText(this.refLink).then().catch(e => console.error(e))
        this.showHideCopy = true
    }

    getReferralLink() {
        apiClient.myReferralLink().then((resp: ApiData<string>) => {
            this.refLink = resp.data
        }).catch((err: any) => {
            console.error('Failed to get referral link: ', err)
        })
    }

    closeNotification() {
        this.dialog = false
        this.checkToMute()
    }

    checkToMute() {
        if (this.muteFuturePopUp) {
            this.$store.commit('muteInviteFriendsPopUp', this.muteFuturePopUp)
        }
    }
    @Watch('$store.state.firstTimeVisitingMatchVideo')
    showNotification() {
        if (this.$store.state.firstTimeVisitingMatchVideo) {
            this.dialog = true
        }
    }

    mounted() {
        this.getReferralLink()
    }
}
</script>