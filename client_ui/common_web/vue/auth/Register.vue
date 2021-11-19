<template>
    <loading-container :is-loading="canRegister === null">
        <template v-slot:default="{ loading }">
            <template v-if="!loading">
                <v-container fluid class="full-parent-height" v-if="canRegister">
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
                        v-model="showHideDuplicateError"
                        :timeout="5000"
                        color="error"
                    >
                        An account with that username or email already exists. Usernames and emails are case-insensitive!
                    </v-snackbar>

                    <v-snackbar
                        v-model="showHideGenericError"
                        :timeout="5000"
                        color="error"
                    >
                        Registration failed. Please try again shortly.
                    </v-snackbar>
                </v-container>

                <div class="full-parent-height d-flex align-center justify-center text-h4 font-weight-bold" v-else>
                    Registrations are temporarily closed. Please try again later!
                </div>
            </template>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { Prop } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { GlobalFlags } from '@client/js/squadov/features'

@Component({
    components: {
        LoadingContainer
    }
})
export default class Register extends Vue {
    @Prop()
    inviteUuid!: string | undefined

    @Prop()
    squadId!: number | undefined

    @Prop()
    sig!: string | undefined

    @Prop({default: undefined})
    redirect!: string | undefined

    @Prop()
    referral!: string | undefined

    canRegister: boolean | null = null
    formValid: boolean = false
    inProgress: boolean = false
    showHideDuplicateError: boolean = false
    showHideGenericError : boolean = false

    email : string = ''
    username: string = ''

    password: string = ''
    confirmPassword: string = ''

    refreshEnabled() {
        apiClient.getGlobalAppFeatures().then((resp: ApiData<GlobalFlags>) => {
            this.canRegister = !resp.data.disableRegistration
        }).catch((err: any) => {
            console.warn('Failed to get if registration is enabled, allowing registrations as fall through: ', err)

            // Generally the advice is to fail secure aka if we fail we'd probably want to disable registrations.
            // That seems like a shitty idea though so I'd rather fail and allow registrations. And honestly
            // chances are that if the previous function fails then any registration isn't going to succeed anyway.
            this.canRegister = true
        })
    }

    mounted() {
        this.refreshEnabled()
    }

    get usernameRules() : any[] {
        return [
            (value : any) => !!value || value.trim().length > 0 || 'Required.',
        ]
    }

    get emailRules() : any[] {
        return [
            (value : any) => !!value || value.trim().length > 0 || 'Required.',
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
            name: 'login',
            query: {
                redirect: this.redirect,
                ref: this.referral,
            }
        }
    }

    register() {
        this.inProgress = true
        apiClient.register({
            username: this.username.trim(),
            password: this.password.trim(),
            email: this.email.trim(),
            ref: this.referral?.trim(),
        }, {
            inviteUuid: this.inviteUuid,
            squadId: !!this.squadId ? this.squadId.toString() : undefined,
            sig: this.sig,
        }).then(() => {
            // Successful registration. Redirect to the login page.
            // Have the login page notify the user their registration succeeded.
            this.$router.push({
                name: 'login',
                query: {
                    reg: this.email,
                    redirect: this.redirect,
                }
            })
        }).catch((err : any) => {
            console.error('Failed to register')
            if (!!err.response?.data.duplicateFlag) {
                this.showHideDuplicateError = true
            } else {
                this.showHideGenericError = true
            }
        }).finally(() => {
            this.inProgress = false
        })
    }
}

</script>

<style scoped>
</style>