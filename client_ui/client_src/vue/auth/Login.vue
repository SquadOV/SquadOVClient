<template>
    <v-container fluid class="full-parent-height">
        <v-row class="full-parent-height" justify="center" align="center">
            <v-alert
                id="registrationAlert"
                v-model="showRegistrationBanner"
                dismissible
                type="success"
            >
                You have successfully registered with SquadOV! Login below to get started.
            </v-alert>

            <v-col cols="8">
                <h1 class="mb-4">Login to SquadOV</h1>

                <v-form
                    v-model="formValid"
                >
                    <v-text-field
                        v-model="username"
                        label="Username/Email"
                        solo
                        :rules="usernameRules"
                        :error-messages="forcedUsernameMessages"
                    >
                    </v-text-field>

                    <v-text-field
                        v-model="password"
                        label="Password"
                        solo
                        :rules="passwordRules"
                        type="password"
                    >
                    </v-text-field>
                </v-form>

                <div class="mb-4">
                    <v-btn
                        :disabled="!formValid"
                        block
                        color="success"
                        @click="login"
                        :loading="inProgress"
                    >
                        Login
                    </v-btn>
                </div>

                <div class="d-flex">
                    <a href="#"  @click="forgotPassword">
                        Forgot your password?
                    </a>

                    <v-spacer></v-spacer>

                    <p>
                        Don't have an account? <router-link :to="registerTo">Register</router-link>.
                    </p>
                </div>
            </v-col>
        </v-row>

        <v-snackbar
            v-model="showHideResetPasswordSuccess"
            :timeout="5000"
            color="success"
        >
            You were sent an email with a link to reset your password if the given username/email exists in our system.
        </v-snackbar>

        <v-snackbar
            v-model="showHideGenericError"
            :timeout="5000"
            color="error"
        >
            Oops! Something went wrong. Please try again shortly.
        </v-snackbar>

        <v-snackbar
            v-model="showHideAuthError"
            :timeout="5000"
            color="error"
        >
            Incorrect username/password. Please try again.
        </v-snackbar>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { ipcRenderer } from 'electron'
import { apiClient, ApiData, LoginOutput } from '@client/js/api'

@Component
export default class Login extends Vue {
    // If we get to the login page and 'reg'
    // is also passed in (non-empty) then that
    // means that the user just successfully registered
    // using this as the username.
    @Prop({default: ''})
    reg!: string
    showRegistrationBanner: boolean = false
    showHideGenericError: boolean = false
    showHideAuthError: boolean = false
    showHideResetPasswordSuccess: boolean = false

    @Watch('reg')
    onSyncReg() {
        if (this.reg === '' ) {
            return
        }

        this.username = this.reg
        this.showRegistrationBanner = true
    }

    formValid: boolean = false
    inProgress: boolean = false
    username: string = ''
    password: string = ''
    forcedUsernameMessages : string[] = []

    get usernameRules() : any[] {
        return [
            (value : any) => !!value || 'Required.',
        ]
    }

    get passwordRules() : any[] {
        return [
            (value : any) => !!value || 'Required.',
        ]
    }

    get registerTo() : any {
        return {
            name: 'register'
        }
    }

    mounted() {
        this.onSyncReg()
    }

    login() {
        apiClient.login({
            username: this.username,
            password: this.password,
        }).then((resp : ApiData<LoginOutput>) => {
            // Successful login - store the session ID.
            ipcRenderer.send('obtain-session', [resp.data.sessionId, resp.data.userId])

            // Forcefully set session ID on the Api client just in case
            // we're going to the email verification check page.
            apiClient.setSessionId(resp.data.sessionId)

            if (!!resp.data.verified) {
                // We can close out the login app now.
                ipcRenderer.send('finish-login')
            } else {
                // Redirect to a screen to wait for email verification.
                this.$router.push({
                    name: 'verify'
                })
            }
        }).catch((err : any) => {
            if (!!err.response && err.response.status === 401) {
                this.showHideAuthError = true
            } else {
                console.log('Login failure: ', err)
                this.showHideGenericError = true
            }
        })
    }

    forgotPassword() {
        // User needs to have put in an email so we know where to send the validation email to.
        if (this.username.length == 0) {
            this.forcedUsernameMessages = ['Username/email required for resetting your password.']
            setTimeout(() => {
                this.forcedUsernameMessages = []
            }, 5000)
            return
        }

        // If the email exists, send a request to the server to send a password reset email.
        apiClient.forgotPassword(this.username).then(() => {
            this.showHideResetPasswordSuccess = true
        }).catch((err : any) => {
            console.log('Failed to reset password: ', err)
            this.showHideGenericError = true
        })
    }
}

</script>

<style scoped>

#registrationAlert {
    position: absolute;
    top: 20px;
}

</style>