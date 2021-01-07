<template>
    <div class="d-flex align-center">
        <v-select
            :value="value"
            @input="$emit('input', arguments[0])"
            label="Valorant Account"
            :items="items"
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
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { RiotAccountData } from '@client/js/valorant/valorant_account'
import { apiClient, ApiData } from '@client/js/api'
import { shell } from 'electron'


@Component
export default class ValorantAccountChooser extends Vue {
    @Prop({required: true})
    value! : RiotAccountData | null

    @Prop({type: Array, required: true})
    options! : RiotAccountData[]

    showHideProgress: boolean = false

    get items() : any[] {
        return this.options!.map((ele : RiotAccountData) => ({
            text: `${ele.gameName}#${ele.tagLine}`,
            value: ele,
        }))
    }

    addAccount() {
        // TODO: Proper RSO login url
        shell.openExternal('https://www.google.com')
        this.showHideProgress = true
    }
}

</script>

<style scoped>

</style>