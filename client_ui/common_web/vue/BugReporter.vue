<template>
    <v-card>
        <template v-if="!$store.state.currentUser.verified">
            <div class="thankyou d-flex justify-center align-center flex-column">
                <div class="text-h5">
                    You have not verified your email address.
                </div>

                <div class="mt-2 mx-8">
                    Without a verified email address, we can not respond to your bug report. Please verify your email address before submitting this report.
                </div>

                <v-btn
                    class="mt-4"
                    color="primary"
                    :to="verifyEmailTo"
                >
                    Verify Email Address
                </v-btn>

                <v-btn
                    class="mt-4"
                    @click="cancel"
                >
                    Nevermind.
                </v-btn>
            </div>
        </template>

        <template v-else-if="!hasCheckedKb">
            <div class="thankyou d-flex justify-center align-center flex-column">
                <div class="text-h5">
                    Have you checked the SquadOV help center?
                </div>

                <div class="mt-2 mx-8">
                    You'll find our FAQ there as well as a ton of help on how to use the app. Check it out! You'll probably find what you're looking for.
                    If not, continue with submitting the report and we'll look into it.
                </div>

                <v-btn
                    class="my-8"
                    color="success"
                    @click="goToHelpCenter"
                >
                    Go to Help Center
                </v-btn>

                <v-btn
                    class="mt-4"
                    color="primary"
                    @click="hasCheckedKb = true"
                >
                    I still need help!
                </v-btn>

                <v-btn
                    class="mt-4"
                    @click="cancel"
                >
                    Nevermind.
                </v-btn>
            </div>
        </template>

        <template v-else-if="!submitted">
            <v-card-title>
                <div>
                    Tell us more!
                </div>

                <v-spacer></v-spacer>

                <div class="d-flex flex-column justify-center align-end">
                    <div>Priority: {{ $store.state.currentUser.supportPriority }}</div>

                    <pricing-notifier-wrapper
                        v-if="!this.$store.getters.isUserInTier(EPricingTier.Gold)"
                        :tier="EPricingTier.Gold"
                        shrink
                    >
                        <span class="text-overline">Want priority support? Upgrade to SquadOV Pro.</span>
                    </pricing-notifier-wrapper>
                </div>
            </v-card-title>

            <v-divider class="my-2"></v-divider>

            <v-form
                v-model="formValid"
                class="ma-4"
            >
                <v-text-field
                    v-model="title"
                    label="Title"
                    filled
                    :rules="titleRules"
                >
                </v-text-field>

                <v-textarea
                    filled
                    label="Description (please explain what bug you encountered and what you were doing when you experienced the bug)"
                    v-model="description"
                    hide-details
                    :rules="descriptionRules"
                >
                </v-textarea>
            </v-form>

            <v-card-actions>
                <v-btn
                    color="error"
                    @click="cancel"
                >
                    Cancel
                </v-btn>

                <v-spacer></v-spacer>

                <v-btn
                    color="success"
                    @click="submit"
                    :loading="inProgress"
                    :disabled="!formValid"
                >
                    Submit
                </v-btn>
            </v-card-actions>
        </template>

        <template v-else>
            <div class="thankyou d-flex justify-center align-center flex-column">
                <div class="text-h5">
                    Thanks for your bug report! We're looking into it!
                </div>

                <div class="mt-2">
                    Did you know that we have a Discord? Let us know you submitted a bug report there and you'll get a faster response (usually).
                </div>

                <v-btn
                    class="mt-4"
                    color="success"
                    @click="joinDiscord"
                >
                    Discord
                </v-btn>

                <v-btn
                    class="mt-1"
                    color="primary"
                    @click="cancel"
                >
                    Close
                </v-btn>
            </div>
        </template>

        <v-snackbar
            v-model="showHideError"
            :timeout="5000"
            color="error"
        >
            Failed to submit the bug report (well that's embarassing). Please try again.
        </v-snackbar>
    </v-card>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { apiClient } from '@client/js/api'
import AdmZip from 'adm-zip'
import fs from 'fs'
import path from 'path'
import process from 'process'
import glob from 'glob'
import { openUrlInBrowser } from '@client/js/external'
import { SettingsPageId } from '@client/js/pages'
import { EPricingTier } from '@client/js/squadov/pricing'
import PricingNotifierWrapper from '@client/vue/utility/squadov/PricingNotifierWrapper.vue'
const MAX_DUMPS_TO_SEND = 5

@Component({
    components: {
        PricingNotifierWrapper
    }
})
export default class BugReporter extends Vue {
    hasCheckedKb: boolean = false
    formValid: boolean = false
    title: string = ''
    description: string = ''
    submitted: boolean = false
    inProgress: boolean = false
    showHideError: boolean = false
    EPricingTier = EPricingTier

    get titleRules() : any[] {
        return [
            (value : any) => !!value || 'Required.',
        ]
    }

    get descriptionRules() : any[] {
        return [
            (value : any) => !!value || 'Required.',
        ]
    }

    get verifyEmailTo(): any {
        return {
            name: SettingsPageId,
            query: {
                inputTab: 2,
            }
        }
    }

    cancel() {
        this.$emit('cancel')
        this.submitted = false
        this.title = ''
        this.description = ''
        this.hasCheckedKb = false
    }

    submit() {
        this.inProgress = true

        let logZip = new AdmZip()
        
        try {
            let appDataFolder = process.env.APPDATA!
            let logDir = path.join(appDataFolder, 'SquadOV' + (!!process.env.SQUADOV_APPDATA_SUFFIX ? process.env.SQUADOV_APPDATA_SUFFIX : ''))    
            let logFiles = glob.sync(`${logDir}/**/*.log`)
            for (let f of logFiles) {
                let data = fs.readFileSync(f)
                let zipFname = path.relative(logDir, f).replace('\\', '/')
                logZip.addFile(zipFname, data)
            }

            let dmpFiles = glob.sync(`${logDir}/**/*.dmp`).map((x: string) => {
                return {
                    fname: x,
                    tm: fs.statSync(x).mtime
                }
            })
            
            dmpFiles.sort((a: any, b: any) => {
                return b.tm.getTime() - a.tm.getTime()
            })

            for (let i = 0; i < Math.min(MAX_DUMPS_TO_SEND, dmpFiles.length); ++i) {
                let data = fs.readFileSync(dmpFiles[i].fname)
                let zipFname = path.relative(logDir, dmpFiles[i].fname).replace('\\', '/')
                logZip.addFile(zipFname, data)
            }
        } catch(e) {
            console.error('Failed to read logs from filesystem: ', e)
            this.inProgress = false
            this.showHideError = true
            return
        }

        console.log('Generating ZIP for logs...')
        let buffer = logZip.toBuffer()
        console.log('Submitting bug report...')
        
        apiClient.submitBugReport(this.title, this.description, new Blob([buffer.buffer])).then(() => {
            this.submitted = true
        }).catch((err: any) => {
            console.error('Failed to submit bug report: ', err)
            this.showHideError = true    
        }).finally(() => {
            this.inProgress = false
        })
    }

    joinDiscord() {
        openUrlInBrowser('https://discord.gg/6Rj5jCVDeC')
    }

    goToHelpCenter() {
        openUrlInBrowser('https://support.squadov.gg')
    }
}

</script>

<style scoped>

.thankyou {
    width: 100%;
    min-height: 300px;
}

</style>