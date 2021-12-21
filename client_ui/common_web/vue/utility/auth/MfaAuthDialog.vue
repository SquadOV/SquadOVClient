<template>
    <div>
        <v-dialog
            v-model="showHide"
            persistent
            max-width="40%"
        >
            <v-card>
                <v-card-title>
                    Enter your 2FA authenticator code
                </v-card-title>
                <v-divider></v-divider>

                <div class="ma-auto" :style="`max-width: ${length * 50}px`">
                    <v-otp-input
                        :length="length"
                        :key="counter"
                        @finish="doVerify"
                    >
                    </v-otp-input>
                </div>

                <div class="d-flex justify-center align-center text-caption" v-if="!useBackupCode">
                    <a href="#" @click="useBackupCode = true">
                        Use Backup Code
                    </a>
                </div>

                <v-divider></v-divider>
                <v-card-actions>
                    <v-btn
                        color="error"
                        @click="hide"
                    >
                        Cancel
                    </v-btn>
                </v-card-actions>
            </v-card>
        </v-dialog>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'

type CallbackFn = (c : string) => void

@Component
export default class MfaAuthDialog extends Vue {
    showHide: boolean = false
    cb: CallbackFn | null = null

    counter: number = 0
    useBackupCode: boolean = false

    get length(): number {
        return this.useBackupCode ? 10 : 6
    }

    hide() {
        this.counter += 1
        this.useBackupCode = false
        this.showHide = false
    }

    doVerify(inCode: string) {
        if (!!this.cb) {
            this.cb(inCode.trim())
        }
        this.hide()
    }

    open(cb: CallbackFn) {
        this.cb = cb
        this.showHide = true
    }

}

</script>