<template>
    <div class="d-flex align-center">
        <v-select
            :value="value"
            @input="$emit('input', arguments[0])"
            label="Summoner"
            :items="items"
            :loading="loading"
            solo
            hide-details
        >
        </v-select>

        <v-btn
            icon
            color="success"
            @click="addAccount"
        >
            <v-icon>
                mdi-plus
            </v-icon>
        </v-btn>

        <v-snackbar
            v-model="showHideProgress"
            :timeout="5000"
            color="success"
        >
            Please login using Riot's login form that was just opened in your web browser.
        </v-snackbar>

        <v-snackbar
            v-model="showError"
            :timeout="5000"
            color="error"
        >
            Something went wrong when redirecting you to Riot's login page. Please submit a bug report!
        </v-snackbar>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { RiotSummoner } from '@client/js/riot/summoner'
import { redirectToRsoLogin } from '@client/js/riot/rso'

@Component
export default class SummonerAccountChooser extends Vue {
    @Prop({required: true})
    value! : RiotSummoner | null

    @Prop({type: Array, required: true})
    options! : RiotSummoner[]

    loading: boolean = false
    showError: boolean = false
    showHideProgress: boolean = false

    get items() : any[] {
        return this.options!.map((ele : RiotSummoner) => ({
            text: ele.summonerName,
            value: ele,
        }))
    }

    addAccount() {
        this.loading = true
        redirectToRsoLogin(this.$store.state.currentUser.id).then(() => {
            this.showHideProgress = true
        }).catch((err: any) => {
            console.log('Failed to redirect to RSO: ', err)
            this.showError = true
        }).finally(() => {
            this.loading = false
        })
    }
}

</script>

<style scoped>

</style>