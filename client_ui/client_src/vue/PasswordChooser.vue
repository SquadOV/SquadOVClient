<template>
    <v-container fluid id="passwordContainer">
        <v-row id="passwordDiv" justify="center" align="center">
            <v-col cols="8">
                <div class="d-flex">
                    <h1 v-if="needCreatePassword">Create a Password</h1>
                    <h1 v-else>Verify your Password</h1>

                    <v-tooltip bottom offset-y>
                        <template v-slot:activator="{on, attrs}">
                            <v-icon v-bind="attrs" v-on="on">
                                mdi-help-circle-outline
                            </v-icon>
                        </template>

                        <span>
                            This password is used to securely encrypt sensitive data locally on your machine.
                            Please select a strong password.
                            Keep this password safe, it can not be reset or recovered and is SEPARATE from your account password.
                        </span>
                    </v-tooltip>
                </div>

                <v-form
                    v-model="formValid"
                >
                    <v-text-field
                        v-model="pwInput"
                        label="Password"
                        solo
                        :rules="passwordRules"
                        type="password"
                    >
                    </v-text-field>

                    <v-text-field
                        v-if="needCreatePassword"
                        v-model="pwConfirm"
                        label="Confirm Password"
                        solo
                        :rules="confirmRules"
                        type="password"
                    >
                    </v-text-field>
                </v-form>

                <div v-if="invalidPassword">
                    <span class="text-h5 red--text">Your input password does not match what was stored.</span>
                </div>

                <div>
                    <v-btn
                        :disabled="!formValid"
                        block
                        color="success"
                        @click="createSubmitPassword"
                        :loading="inProgress"
                    >
                        Submit
                    </v-btn>
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

import bcryptjs from 'bcryptjs'
import path from 'path'
import fs from 'fs'

const saltRounds = 10

@Component
export default class PasswordChooser extends Vue {
    pwHash : string | null = null
    formValid: boolean = false

    pwInput: string = ''
    pwConfirm: string = ''
    invalidPassword: boolean = false
    inProgress: boolean = false

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
            (value : any) => (this.pwInput === value) || 'Password does not match.',
        ]
    }

    get needCreatePassword() : boolean {
        return !this.pwHash
    }


    onSuccess(pw : string) {
        ipcRenderer.send('password', pw)
    }

    onError(err : any) {
        console.log(err)
        window.close()
    }

    get hashFname() : string {
        let folder = ipcRenderer.sendSync('request-app-folder')
        if (!folder) {
            this.onError('Failed to get user app folder.')
            return ''
        }
        return path.join(folder, 'verify.bcrypt')
    }

    reloadPasswordHash() {
        // Try to read the base64 encoded hash from a file.
        // If it does exist, then we only ask user to verify
        // the password. If it doesn't exist, then ask user to
        // create and confirm a password.
        if (!fs.existsSync(this.hashFname)) {
            this.pwHash = null
            return
        }

        this.pwHash = fs.readFileSync(this.hashFname, {encoding: 'utf-8'})
    }

    createSubmitPassword() {
        this.inProgress = true
        if (this.needCreatePassword) {
            let hash = bcryptjs.hashSync(this.pwInput, saltRounds)

            // Need to create directory if it doesn't exist.
            const dir = path.dirname(this.hashFname)
            if (!fs.existsSync(dir)) {
                fs.mkdirSync(dir, {
                    recursive: true,
                })
            }

            fs.writeFileSync(this.hashFname, hash, {
                encoding: 'utf-8',
                mode: 0o644,
            })

            this.onSuccess(this.pwInput)
        } else {
            if (!bcryptjs.compareSync(this.pwInput, this.pwHash)) {
                this.inProgress = false
                this.invalidPassword = true
                this.pwInput = ''
                return
            }

            this.onSuccess(this.pwInput)
        }
    }

    mounted() {
        this.reloadPasswordHash()
    }
}

</script>

<style scoped>

#passwordContainer {
    height: 100%;
}

#passwordDiv {
    height: 100%;
}

</style>