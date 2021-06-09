<template>
    <div>
        <v-text-field
            :value="link"
            hide-details
            single-line
            outlined
            dense
            readonly
            ref="urlInput"
        >
            <template v-slot:append-outer>
                <v-btn
                    icon
                    color="success"
                    @click="doCopy"
                >
                    <v-icon>
                        mdi-content-copy
                    </v-icon>
                </v-btn>
            </template>
        </v-text-field>

        <v-snackbar
            v-model="showHideCopy"
            :timeout="5000"
            color="success"
        >
            Copied URL to clipboard!
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api' 

@Component
export default class ReferralLink extends Vue {
    link: string | null = null
    showHideCopy: boolean = false

    $refs!: {
        urlInput: any
    }

    @Watch('$store.state.currentUser.username')
    refreshReferral() {
        apiClient.myReferralLink().then((resp: ApiData<string>) => {
            this.link = resp.data
        }).catch((err: any) => {
            console.log('Failed to get referral link: ', err)
        })
    }

    mounted() {
        this.refreshReferral()
    }

    doCopy() {
        let inputEle = this.$refs.urlInput.$el.querySelector('input')
        inputEle.select()
        document.execCommand('copy')
        this.showHideCopy = true
    }
}

</script>