<template>
    <div>
        <v-btn
            icon
            :loading="loading"
            @click="toggleWatchlist"
        >   
            <v-icon v-if="isWatchlist">
                mdi-playlist-check
            </v-icon>

            <v-icon v-else>
                mdi-playlist-plus
            </v-icon>
        </v-btn>

        <v-snackbar
            v-model="showHideError"
            :timeout="5000"
            color="error"
        >
            Oops! Something went wrong when adding this VOD to your watchlist, try again later!
        </v-snackbar>
    </div>
</template>


<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'

@Component
export default class VodWatchlistButton extends Vue {
    @Prop({required: true})
    vodUuid!: string

    isWatchlist: boolean = false
    loading: boolean = false
    showHideError: boolean = false

    toggleWatchlist() {
        this.loading = true

        let promise
        if (this.isWatchlist) {
            promise = apiClient.removeVodWatchlist(this.vodUuid)
        } else {
            promise = apiClient.addVodWatchlist(this.vodUuid)
        }

        promise.then(() => {
            this.isWatchlist = !this.isWatchlist
        }).catch((err: any) => {
            console.log('Failed to toggle VOD watch list: ', err)
            this.showHideError = true
        }).finally(() => {
            this.loading = false
        })
    }

    @Watch('vodUuid')
    resyncStatus() {
        this.loading = true
        apiClient.isVodWatchlist(this.vodUuid).then((resp: ApiData<boolean>) => {
            this.isWatchlist = resp.data
        }).catch((err: any) => {
            console.log('Failed to check if VOD is on watchlist: ', err)
        }).finally(() => {
            this.loading = false
        })
    }

    mounted() {
        this.resyncStatus()
    }
}

</script>