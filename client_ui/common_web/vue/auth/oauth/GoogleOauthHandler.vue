<template>
    <div class="d-flex flex-column justify-center align-center full-parent-height full-width">
        <div class="text-h6" v-if="!showSuccess">
            Click below to connect your Google account to SquadOV:
        </div>
        <div class="mb-6" v-if="!showSuccess">
            <div class="g-signin2" ref="googleLoginBtn" id="googleBtn">
                
            </div>
        </div>
        <template v-if="showSuccess">
            <div class="my-2">
                Success! You may now close this window.
            </div>
            <v-btn
                @click="redirect"
                color="success"
            >
                Continue to SquadOV
            </v-btn>
        </template>

        <v-snackbar
            v-model="showError"
            :timeout="5000"
            color="error"
        >
            Something went wrong when authenticating your Google account. Please submit a bug report and try again!
        </v-snackbar>
    </div>
</template>

<script lang="ts">
declare var google: any;

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'

@Component
export default class GoogleOauthHandler
 extends Vue {
    @Prop({required: true})
    state!: string

    @Prop({required: true})
    code!: string

    showError: boolean = false
    showSuccess: boolean = false
    redirectCounter: number = 5
    token: string = ""

    redirect() {
        window.location.href = `sqdOv://login#${this.token}`
    }

    mounted() {
        let googleSignInAPI = document.createElement('script')
        googleSignInAPI.setAttribute('src', 'https://accounts.google.com/gsi/client')
        document.head.appendChild(googleSignInAPI)
        googleSignInAPI.onload = this.InitGoogleButton
    }
    InitGoogleButton() {
        const cID = process.env.SQUADOV_GOOGLE_CLIENT_ID
        const credFunc = this.onSignIn
        window.onload = function () {
          google.accounts.id.initialize({
            client_id: cID,
            callback: credFunc
          });
          google.accounts.id.renderButton(
            document.getElementById("googleBtn"),
            { theme: "outline", size: "large" }
          );
        }
    }
    onSignIn(response: any) {
        this.token = response.credential
        this.showSuccess = true
        window.location.href = `sqdOv://login#${response.credential}`
    }
}

</script>