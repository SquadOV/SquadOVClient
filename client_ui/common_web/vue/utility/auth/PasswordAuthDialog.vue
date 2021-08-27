<template>
    <div>
        <v-dialog
            v-model="showHide"
            persistent
            max-width="40%"
        >
            <v-card>
                <v-card-title>
                    Confirm your Password
                </v-card-title>
                <v-divider></v-divider>

                <v-form
                    v-model="passwordValid"
                >
                    <v-text-field
                        v-model="password"
                        label="Password"
                        solo
                        :rules="rules"
                        type="password"
                    >
                    </v-text-field>
                </v-form>

                <v-divider></v-divider>
                <v-card-actions>
                    <v-btn
                        color="error"
                        @click="showHide = false"
                    >
                        Cancel
                    </v-btn>
                    <v-spacer></v-spacer>
                    <v-btn
                        color="success"
                        @click="doVerify"
                        :loading="inProgress"
                        :disabled="!passwordValid"
                    >
                        Verify
                    </v-btn>
                </v-card-actions>
            </v-card>
        </v-dialog>

        <v-snackbar
            v-model="pwError"
            :timeout="5000"
            color="error"
        >
            Failed to verify your password. Double check your password and try again.
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import { apiClient, ApiData } from '@client/js/api'
import Vue from 'vue'
import Component from 'vue-class-component'

type CallbackFn = () => void

@Component
export default class PasswordAuthDialog extends Vue {
    showHide: boolean = false
    cb: CallbackFn | null = null

    passwordValid: boolean = false
    inProgress: boolean = false
    password: string = ''
    pwError: boolean = false

    open(cb: CallbackFn) {
        this.cb = cb
        this.showHide = true
        this.password = ''
    }

    get rules(): any[] {
        return [
            (value : any) => !!value || 'Required.',
        ]
    }

    doVerify() {
        this.inProgress = true
        apiClient.verifyPassword(this.password).then(() => {
            if (!!this.cb) {
                this.cb()
            }
            this.showHide = false
        }).catch((err: any) => {
            console.warn('Failed to verify password.')
            this.pwError = true
        }).finally(() => {
            this.inProgress = false
            this.password = ''
        })
    }
}

</script>