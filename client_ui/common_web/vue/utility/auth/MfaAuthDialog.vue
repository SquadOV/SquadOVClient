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

                <div class="ma-auto" style="max-width: 300px">
                    <v-otp-input
                        v-model="code"
                        length="6"
                        type="number"
                        @finish="doVerify"
                    >
                    </v-otp-input>
                </div>

                <v-divider></v-divider>
                <v-card-actions>
                    <v-btn
                        color="error"
                        @click="showHide = false"
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

    code: string = ''

    doVerify() {
        if (!!this.cb) {
            this.cb(this.code.trim())
        }
        this.showHide = false
    }

    open(cb: CallbackFn) {
        this.cb = cb
        this.showHide = true
    }

}

</script>