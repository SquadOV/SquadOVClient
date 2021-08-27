<template>
    <div>
        <div class="d-flex align-center">
            <span class="text-overline font-weight-bold mr-4">Password</span>
        </div>
        <v-divider></v-divider>
        
        <div class="mt-2">
            <v-form
                v-model="passwordValid"
            >
                <v-text-field
                    v-model="currentPassword"
                    label="Current Password"
                    solo
                    :rules="currentRules"
                    type="password"
                >
                </v-text-field>

                <v-text-field
                    v-model="newPassword"
                    label="New Password"
                    solo
                    :rules="passwordRules"
                    type="password"
                >
                </v-text-field>

                <v-text-field
                    v-model="passwordConfirm"
                    label="Confirm new Password"
                    solo
                    :rules="confirmRules"
                    type="password"
                >
                </v-text-field>
            </v-form>

            <div class="d-flex align-center">
                <v-btn color="warning" :loading="passwordChanging" @click="changePassword" :disabled="!passwordValid">
                    Change Password
                </v-btn>

                <div class="ml-2" v-if="passwordSuccess">
                    Password Changed Successfully.
                </div>
            </div>
        </div>

        <div class="d-flex align-center mt-4">
            <span class="text-overline font-weight-bold mr-4">Two-Factor Authentication</span>
        </div>
        <v-divider></v-divider>

        <loading-container :is-loading="is2faEnabled === null">
            <template v-slot:default="{ loading }">
                <div class="mt-2" v-if="!loading">
                    <div v-if="is2faEnabled">
                        <v-icon size="small" color="success">
                            mdi-lock
                        </v-icon>

                        Enabled

                        <v-btn class="ml-1" color="error" @click="requestDisable2FA" :loading="progress2fa">
                            Remove 2FA
                        </v-btn>
                    </div>

                    <div v-else>
                        <v-icon size="small" color="error">
                            mdi-lock-open
                        </v-icon>

                        Disabled

                        <v-btn class="ml-1" color="success" @click="requestEnable2FA" :loading="progress2fa">
                            Enable 2FA
                        </v-btn>
                    </div>
                </div>
            </template>
        </loading-container>

        <v-snackbar
            v-model="passwordFail"
            :timeout="5000"
            color="error"
        >
            Password change failed. Double check your current password and try again.
        </v-snackbar>

        <v-snackbar
            v-model="error2fa"
            :timeout="5000"
            color="error"
        >
            Something went wrong processing your 2FA request. Please try again.
        </v-snackbar>

        <password-auth-dialog ref="password">
        </password-auth-dialog>

        <enable-mfa-dialog ref="mfa">
        </enable-mfa-dialog>

        <mfa-auth-dialog ref="mfaAuth">
        </mfa-auth-dialog>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import PasswordAuthDialog from '@client/vue/utility/auth/PasswordAuthDialog.vue'
import EnableMfaDialog from '@client/vue/utility/auth/EnableMfaDialog.vue'
import MfaAuthDialog from '@client/vue/utility/auth/MfaAuthDialog.vue'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { apiClient, ApiData } from '@client/js/api'
import { MfaData } from '@client/js/squadov/mfa'

@Component({
    components: {
        PasswordAuthDialog,
        EnableMfaDialog,
        MfaAuthDialog,
        LoadingContainer,
    }
})
export default class AccountSecuritySettingsItems extends Vue {
    is2faEnabled: boolean | null = null
    passwordValid: boolean = false
    currentPassword: string = ''
    newPassword: string = ''
    passwordConfirm: string = ''
    passwordChanging: boolean = false

    passwordFail: boolean = false
    passwordSuccess: boolean = false

    progress2fa: boolean = false
    error2fa: boolean = false

    $refs!: {
        password: PasswordAuthDialog
        mfa: EnableMfaDialog
        mfaAuth: MfaAuthDialog
    }

    mounted() {
        apiClient.check2faStatus().then((resp: ApiData<boolean>) => {
            this.is2faEnabled = resp.data
        }).catch((err: any) => {
            console.error('Failed to check 2FA status: ', err)
        })
    }

    get currentRules(): any[] {
        return [
            (value : any) => !!value || 'Required.',
        ]
    }

    get passwordRules() : any[] {
        return [
            (value : any) => !!value || 'Required.',
            (value : any) => (value || '').length >= 8 || 'Password must have at least 8 characters.',
            (value : any) => (value || '').length <= 20 || 'Password can have at most 20 characters.',
        ]
    }

    get confirmRules(): any[] {
        return [
            (value : any) => !!value || 'Required.',
            (value : any) => (this.newPassword === value) || 'Password does not match.',
        ]
    }
    
    changePassword() {
        this.passwordChanging = true
        apiClient.changePassword(this.currentPassword, this.newPassword).then(() => {
            this.currentPassword = ''
            this.newPassword = ''
            this.passwordConfirm = ''
            this.passwordSuccess = true
            setTimeout(() => {
                this.passwordSuccess = false
            }, 5000)
        }).catch((err: any) => {
            console.error('Failed to change user password.')
            this.passwordFail = true
            this.currentPassword = ''
        }).finally(() => {
            this.passwordChanging = false
        })
    }

    requestEnable2FA() {
        this.$refs.password.open(() => {
            this.progress2fa = true
            apiClient.request2FAQrCodeUrl().then((resp: ApiData<MfaData>) => {
                // Display a QR code and let the user give us back the proper code to enable 2FA.
                this.$refs.mfa.open(resp.data.uri, resp.data.secret.secretBase32Encoded, () => {
                    // At this point we've successfully verified that the user has enabled 2FA.
                    this.is2faEnabled = true
                })
            }).catch((err: any) => {
                console.error('Failed to request 2FA url: ', err)
                this.error2fa = true
            }).finally(() => {
                this.progress2fa = false
            })
        })
    }

    requestDisable2FA() {
        this.$refs.mfaAuth.open((code: string) => {
            this.progress2fa = true
            apiClient.disable2fa(code).then(() => {
                this.is2faEnabled = false
            }).catch((err: any) => {
                console.error('Failed to disable 2FA: ', err)
                this.error2fa = true
            }).finally(() => {
                this.progress2fa = false
            })
        })
    }
}

</script>