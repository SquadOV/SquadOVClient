<template>
    <div class="full-width">
        <div class="d-flex align-center ma-4">
            <v-text-field
                v-model="vodSearch"
                label="Video ID"
                solo
                dense
                hide-details
            >
            </v-text-field>

            <span class="mx-4">OR</span>

            <v-text-field
                v-model="userSearch"
                label="User ID"
                solo
                dense
                hide-details
            >
            </v-text-field>

            <v-btn
                class="ml-4"
                color="primary"
                :disabled="!hasSearch"
                @click="searchVod"
                :loading="searching"
            >
                Search
            </v-btn>
        </div>

        <div class="text-h6 font-weight-bold">
            Last 20 VODs
        </div>

        <div v-if="vods.length > 0">
            <vod-display
                class="mb-4"
                v-for="(v, idx) in vods"
                :key="idx"
                :vod="v"
            >
            </vod-display>
        </div>

        <div class="text-h2" v-else>
            No VODs Found
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import VodDisplay from '@client/vue/vod/VodDisplay.vue'
import { Vod, searchForVod } from '@client/ts/vod'

@Component({
    components: {
        VodDisplay
    }
})
export default class SearchVod extends Vue {
    vodSearch: string = ''
    userSearch: string = ''
    searching: boolean = false
    vods: Vod[] = []

    get hasSearch(): boolean {
        if (this.vodSearch.trim() !== '') {
            return true
        }

        if (this.userSearch.trim() !== '') {
            return true
        }

        return false
    }

    searchVod() {
        let vsearch = this.vodSearch.trim()
        let usearch = this.userSearch.trim()

        this.searching = true
        searchForVod(vsearch, usearch).then((v: Vod[]) => {
            this.vods = v
        }).catch((err: any) => {
            console.log('Failed to find VODs: ', err)
        }).finally(() => {
            this.searching = false
        })
    }
}

</script>
