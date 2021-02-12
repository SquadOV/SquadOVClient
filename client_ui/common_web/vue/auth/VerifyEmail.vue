<template>
    <div id="container" class="d-flex flex-column align-center justify-center">
        <v-row justify="center" align="center" v-if="verified === null">
            <v-progress-circular size="64" indeterminate>
            </v-progress-circular>
        </v-row>

        <div id="responseContainer" class="text-center" v-else>
            <div class="text-h3" v-if="!verified">
                Oh no! Something went wrong.
            </div>

            <div class="d-flex justify-center align-center flex-column" v-else>
                <div class="text-h5 font-weight-bold">
                    Email verification successful. Redirecting you to SquadOV in {{ redirectSeconds }} seconds...
                </div>

                <v-btn
                    class="mt-4"
                    color="success"
                    @click="redirect"
                >
                    Redirect me now!
                </v-btn>
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient } from '@client/js/api'
import * as pi from '@client/js/pages'

@Component
export default class VerifyEmail extends Vue {
    @Prop({required: true})
    verificationId!: string

    verified : boolean | null = null
    redirectSeconds: number = 5
    itvl: number = 0

    redirect() {
        this.$router.replace({
            name: pi.LoginPageId
        })
    }

    @Watch('verificationId')
    performVerification() {
        apiClient.verifyEmail(this.verificationId).then(() => {
            this.verified = true
            this.redirectSeconds = 5
            this.itvl = window.setInterval(() => {
                this.redirectSeconds -= 1
                if (this.redirectSeconds <= 0) {
                    this.redirect()
                }
            }, 1000)
        }).catch((err : any) => {
            this.verified = false
        })
    }

    beforeDestroy() {
        if (!!this.itvl) {
            window.clearInterval(this.itvl)
        }
    }

    mounted() {
        this.performVerification()
    }
}

</script>


<style scoped>

#container {
    height: 100vh;
}

#responseContainer {
    width: 60%;
}

</style>