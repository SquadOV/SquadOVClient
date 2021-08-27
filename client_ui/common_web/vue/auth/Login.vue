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
                    @submit.prevent="login"
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

                    <div class="mb-4">
                        <v-btn
                            :disabled="!formValid"
                            block
                            color="success"
                            :loading="inProgress"
                            type="submit"
                        >
                            Login
                        </v-btn>
                    </div>
                </v-form>

                <div class="d-flex">
                    <a href="#"  @click="forgotPassword" v-if="!forgotInProgress">
                        Forgot your password?
                    </a>

                    <v-progress-circular indeterminate size="16" v-else>
                    </v-progress-circular>

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
            Incorrect username/password (or two-factor code). Please try again.
        </v-snackbar>

        <mfa-auth-dialog ref="mfa"></mfa-auth-dialog>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import * as pi from '@client/js/pages'

/// #if DESKTOP
import { ipcRenderer } from 'electron'
/// #endif

import { apiClient, ApiData, LoginOutput } from '@client/js/api'
import MfaAuthDialog from '@client/vue/utility/auth/MfaAuthDialog.vue'

@Component({
    components: {
        MfaAuthDialog
    }
})
export default class Login extends Vue {
    // If we get to the login page and 'reg'
    // is also passed in (non-empty) then that
    // means that the user just successfully registered
    // using this as the username.
    @Prop({default: ''})
    reg!: string

    @Prop({default: undefined})
    redirect!: string | undefined

    @Prop()
    referral!: string | undefined

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
    forgotInProgress: boolean = false
    username: string = ''
    password: string = ''
    forcedUsernameMessages : string[] = []

    $refs!: {
        mfa: MfaAuthDialog
    }

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
            name: 'register',
            query: {
                redirect: this.redirect,
                ref: this.referral,
            }
        }
    }

    mounted() {
        this.onSyncReg()
    }
    
    handleSuccessfulLogin(data: LoginOutput) {
        // Successful login - store the session ID.
/// #if DESKTOP
        ipcRenderer.send('obtain-session', [data.sessionId, data.userId])
/// #endif
        // Forcefully set session ID on the Api client just in case
        // we're going to the email verification check page.
        apiClient.setSessionFull(data.sessionId, data.userId)

        // We can close out the login app now.
/// #if DESKTOP
        ipcRenderer.send('finish-login')
/// #else
        if (!!this.redirect) {
            this.$router.replace(this.redirect)
        } else {
            this.$router.replace({
                name: pi.DashboardPageId,
            })
        }
/// #endif
    }

    login() {
        this.inProgress = true
        apiClient.login({
            username: this.username,
            password: this.password,
        }).then((resp : ApiData<LoginOutput>) => {
            if (!!resp.data.twoFactor) {
                // Ask for two factor code before passing that back to the server to finish the user login.
                this.$refs.mfa.open((code: string) => {
                    apiClient.finishMfaLogin(resp.data.twoFactor!, code).then((r2: ApiData<LoginOutput>) => {
                        this.handleSuccessfulLogin(r2.data)
                    }).catch((err: any) => {
                        console.warn('MFA Login Failure')
                        this.showHideAuthError = true
                    })
                })
            } else {
                this.handleSuccessfulLogin(resp.data)
            }
        }).catch((err : any) => {
            if (!!err.response && err.response.status === 401) {
                this.showHideAuthError = true
            } else {
                console.warn('Login failure')
                this.showHideGenericError = true
            }
        }).finally(() => {
            this.inProgress = false
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

        this.forgotInProgress = true
        // If the email exists, send a request to the server to send a password reset email.
        apiClient.forgotPassword(this.username).then(() => {
            this.showHideResetPasswordSuccess = true
        }).catch((err : any) => {
            console.error('Failed to reset password: ', err)
            this.showHideGenericError = true
        }).finally(() => {
            this.forgotInProgress = false
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