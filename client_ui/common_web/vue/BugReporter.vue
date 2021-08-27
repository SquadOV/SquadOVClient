<template>
    <v-card>
        <template v-if="!submitted">
            <v-card-title>
                Tell us more!
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
            <div class="thankyou d-flex justify-center align-center text-h5">
                Thanks for your bug report! We're looking into it!
            </div>

            <v-card-actions>
                <v-spacer></v-spacer>
                <v-btn
                    color="primary"
                    @click="cancel"
                >
                    Close
                </v-btn>
            </v-card-actions>
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

const MAX_DUMPS_TO_SEND = 5

@Component
export default class BugReporter extends Vue {
    formValid: boolean = false
    title: string = ''
    description: string = ''
    submitted: boolean = false
    inProgress: boolean = false
    showHideError: boolean = false

    get titleRules() : any[] {
        return [
            (value : any) => !!value || 'Required.',
        ]
    }

    cancel() {
        this.$emit('cancel')
        this.submitted = false
        this.title = ''
        this.description = ''
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
}

</script>

<style scoped>

.thankyou {
    width: 100%;
    min-height: 300px;
}

</style>