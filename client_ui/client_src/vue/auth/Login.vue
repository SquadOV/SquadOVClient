<template>
    <v-container fluid class="full-parent-height">
        <v-row class="full-parent-height" justify="center" align="center">
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
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import { ipcRenderer } from 'electron'

@Component
export default class Login extends Vue {
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

    login() {}

    forgotPassword() {
        // User needs to have put in an email so we know where to send the validation email to.
        if (this.username.length == 0) {
            this.forcedUsernameMessages = ['Username/email required for resetting your password.']
            setTimeout(() => {
                this.forcedUsernameMessages = []
            }, 5000)
            return
        }

        // If the email exists, send a request to the server.
    }
}

</script>

<style scoped>

</style>