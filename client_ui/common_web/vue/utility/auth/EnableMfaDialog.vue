<template>
    <div>
        <v-dialog
            v-model="showHide"
            persistent
            max-width="40%"
        >
            <v-card v-if="!showBackups">
                <v-card-title>
                    Enable 2FA for your Account
                </v-card-title>
                <v-divider></v-divider>

                <div class="pa-4">
                    Scan the following QR code in an authenticator app such as Google Authenticator.
                    Afterwards, enter the code into the following textbox and hit 'Submit'.

                    <div class="d-flex justify-center align-center my-4">
                        <canvas ref="qrcode"></canvas>
                    </div>

                    <v-text-field
                        v-model="code"
                        label="Code"
                        solo
                    >
                    </v-text-field>
                </div>

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
                        @click="enable2fa"
                        :loading="inProgress"
                        :disabled="code.length === 0"
                    >
                        Submit
                    </v-btn>
                </v-card-actions>
            </v-card>

            <v-card v-else>
                <v-card-title>
                    2FA Backup Codes
                </v-card-title>
                <v-divider></v-divider>

                <div class="pa-4">
                    Save these codes securely as they may be used in place of your authenticator code in case you lose access to your authenticator.

                    <pre class="my-4">{{ backupCodes.join('\n') }}</pre>
                </div>

                <v-divider></v-divider>
                <v-card-actions>
                    <v-spacer></v-spacer>
                    <v-btn
                        color="success"
                        @click="onAck"
                    >
                        OK
                    </v-btn>
                </v-card-actions>
            </v-card>
        </v-dialog>

        <v-snackbar
            v-model="error"
            :timeout="5000"
            color="error"
        >
            An error occurred while verifying your 2FA code, please try again.
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import { apiClient, ApiData } from '@client/js/api'
import QRCode from 'qrcode'
import Vue from 'vue'
import Component from 'vue-class-component'

type CallbackFn = () => void

@Component
export default class EnableMfaDialog extends Vue {
    showHide: boolean = false
    showBackups: boolean = false
    cb: CallbackFn | null = null

    secret: string = ''
    code: string = ''
    inProgress: boolean = false
    error: boolean = false
    backupCodes: string[] = []

    $refs!: {
        qrcode: HTMLCanvasElement
    }

    open(url: string, secret: string, cb: CallbackFn) {
        this.cb = cb
        this.showHide = true
        this.showBackups = false
        this.code = ''
        this.secret = secret
        this.backupCodes = []

        Vue.nextTick(() => {
            QRCode.toCanvas(this.$refs.qrcode, url, (err: any) => {
                if (!!err) {
                    console.error('Failed to generate QR code: ', err)
                    this.error = true
                }
            })
        })
    }

    enable2fa() {
        this.inProgress = true
        apiClient.enable2fa(this.code, this.secret).then((resp: ApiData<string[]>) => {
            this.backupCodes = resp.data
            this.showBackups = true
        }).catch((err: any) => {
            console.error('Failed to enable 2fa: ', err)
            this.error = true
        }).finally(() => {
            this.inProgress = false
        })
    }

    onAck() {
        if (!!this.cb) {
            this.cb()
        }
        this.showHide = false
    }
}

</script>