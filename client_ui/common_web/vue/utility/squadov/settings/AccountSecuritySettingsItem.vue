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

        <div class="mt-2">
            <div v-if="!!$store.state.currentUser.enabled2fa">
                <v-icon size="small" color="success">
                    mdi-lock
                </v-icon>

                Enabled

                <v-btn class="ml-1" color="error">
                    Remove 2FA
                </v-btn>
            </div>

            <div v-else>
                <v-icon size="small" color="error">
                    mdi-lock-open
                </v-icon>

                Disabled

                <v-btn class="ml-1" color="success">
                    Enable 2FA
                </v-btn>
            </div>
        </div>

        <v-snackbar
            v-model="passwordFail"
            :timeout="5000"
            color="error"
        >
            Password change failed. Double check your current password and try again.
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { apiClient } from '@client/js/api'

@Component
export default class AccountSecuritySettingsItems extends Vue {
    passwordValid: boolean = false
    currentPassword: string = ''
    newPassword: string = ''
    passwordConfirm: string = ''
    passwordChanging: boolean = false

    passwordFail: boolean = false
    passwordSuccess: boolean = false

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
            console.log('Failed to change user password.')
            this.passwordFail = true
            this.currentPassword = ''
        }).finally(() => {
            this.passwordChanging = false
        })
    }
}

</script>