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

const saltRounds = 10

@Component
export default class Register extends Vue {
    formValid: boolean = false
    inProgress: boolean = false
    showHideGenericError : boolean = false

    email : string = ''
    username: string = ''

    password: string = ''
    confirmPassword: string = ''

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

    register() {
        this.inProgress = true
        apiClient.register({
            username: this.username,
            password: this.password,
            email: this.email,
        }).then(() => {
            // Successful registration. Redirect to the login page.
            // Have the login page notify the user their registration succeeded.
            this.$router.push({
                name: 'login',
                query: {
                    reg: this.email,
                }
            })
        }).catch((err : any) => {
            console.log('Failed to register')
            this.showHideGenericError = true
        }).finally(() => {
            this.inProgress = false
        })
    }
}

</script>

<style scoped>
</style>