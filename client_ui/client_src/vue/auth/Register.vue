<template>
    <v-container fluid class="full-parent-height">
        <v-row class="full-parent-height" justify="center" align="center">
            <v-col cols="8">
                <h1 class="mb-4">Register with SquadOV</h1>

                <v-form
                    v-model="formValid"
                >
                    <v-text-field
                        v-model="username"
                        label="Username"
                        solo
                        :rules="usernameRules"
                    >
                    </v-text-field>

                    <v-text-field
                        v-model="email"
                        label="Email"
                        solo
                        :rules="emailRules"
                    >
                    </v-text-field>

                    <v-text-field
                        v-if="!!pwHash"
                        v-model="oldPassword"
                        label="Old Password"
                        solo
                        :rules="oldPasswordRules"
                        type="password"
                        :error-messages="oldPwErrors"
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

                    <v-text-field
                        v-model="confirmPassword"
                        label="Confirm Password"
                        solo
                        :rules="confirmRules"
                        type="password"
                    >
                    </v-text-field>
                </v-form>

                <div class="mb-4">
                    <v-btn
                        :disabled="!formValid"
                        block
                        color="success"
                        @click="register"
                        :loading="inProgress"
                    >
                        Register
                    </v-btn>
                </div>

                <div class="d-flex">
                    <p>
                        Already have an account? <router-link :to="loginTo">Login</router-link>.
                    </p>
                </div>
            </v-col>
        </v-row>

        <v-snackbar
            v-model="showHideGenericError"
            :timeout="5000"
            color="error"
        >
            Registration failed. Please try again shortly.
        </v-snackbar>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import { apiClient } from '@client/js/api'
import bcryptjs from 'bcryptjs'
import path from 'path'
import fs from 'fs'

const saltRounds = 10

@Component
export default class Register extends Vue {
    formValid: boolean = false
    inProgress: boolean = false
    showHideGenericError : boolean = false

    email : string = ''
    username: string = ''

    oldPassword: string = ''
    oldPwErrors : string[] = []

    password: string = ''
    confirmPassword: string = ''

    @Watch('oldPassword')
    changeOldPassword() {
        this.oldPwErrors = []
    }

    get usernameRules() : any[] {
        return [
            (value : any) => !!value || 'Required.',
        ]
    }

    get emailRules() : any[] {
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

    get oldPasswordRules(): any[] {
        return [
            (value : any) => !!value || 'Required.',
        ]
    }

    get confirmRules(): any[] {
        return [
            (value : any) => !!value || 'Required.',
            (value : any) => (this.password === value) || 'Password does not match.',
        ]
    }

    get loginTo() : any {
        return {
            name: 'login'
        }
    }

    get hashFname() : string {
        if (!process.env.APPDATA) {
            // Um?? Not windows. Derp.
            console.log('Failed to find APPDATA env variable.')
            return ''
        }
        return path.join(process.env.APPDATA, 'SquadOV', 'verify.bcrypt')
    }

    get pwHash() : string | null {
        if (!fs.existsSync(this.hashFname)) {
            return null
        }

        return fs.readFileSync(this.hashFname, {encoding: 'utf-8'})
    }

    register() {
        // When we register we need to check for a legacy local password. In that case
        // we need to check that the user has it input correctly so we can decrypt
        // things that need to be decrypted. Then after successful registration, we
        // need to re-encrypt using our new encryption key.
        if (!!this.pwHash && !bcryptjs.compareSync(this.oldPassword, this.pwHash)) {
            this.oldPwErrors = [
                'Incorrect old password.'
            ]
            setTimeout(() => {
                this.oldPwErrors = []
            }, 5000)
            return
        }
        
        apiClient.register({
            username: this.username,
            password: this.password,
            email: this.email,
        }).then(() => {
            if (!!this.pwHash) {
                // Re-encrypt all the dataaaaaaa.
            }

            // Successful registration. Redirect to the login page.
            // Have the login page notify the user their registration succeeded.
            this.$router.push({
                name: 'login',
                query: {
                    reg: this.email,
                }
            })
        }).catch((err : any) => {
            console.log('Failed to register: ', err)
            this.showHideGenericError = true
        })
    }
}

</script>

<style scoped>
</style>