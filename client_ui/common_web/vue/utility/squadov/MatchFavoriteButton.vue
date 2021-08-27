<template>
    <div v-if="canFavorite">
        <v-dialog
            v-model="showHideReason"
            persistent
            max-width="60%"
        >
            <v-card>
                <v-card-title>
                    Why is this match awesome?
                </v-card-title>

                <div class="pa-4">
                    <v-text-field
                        v-model="reason"
                        label="Remind yourself why it's awesome!"
                        filled
                    >
                    </v-text-field>
                </div>

                <v-card-actions>
                    <v-btn
                        color="error"
                        @click="showHideReason = false"
                    >
                        Cancel
                    </v-btn>

                    <v-spacer></v-spacer>

                    <v-btn
                        color="success"
                        @click="toggleFavorite"
                    >
                        Favorite
                    </v-btn>
                </v-card-actions>
            </v-card>
        </v-dialog>

        <v-tooltip bottom :disabled="!isFavorite">
            <template v-slot:activator="{on, attrs}">
                <v-btn
                    v-bind="attrs"
                    v-on="on"
                    icon
                    :loading="loading"
                    @click="startFavoriteFlow"
                    color="warning"
                >   
                    <v-icon v-if="isFavorite">
                        mdi-star
                    </v-icon>

                    <v-icon v-else>
                        mdi-star-outline
                    </v-icon>
                </v-btn>
            </template>

            {{ reason }}
        </v-tooltip>

        <v-snackbar
            v-model="showHideError"
            :timeout="5000"
            color="error"
        >
            Oops! Something went wrong when favoriting this match, try again later!
        </v-snackbar>
    </div>
</template>


<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { MatchFavoriteResponse } from '@client/js/squadov/recentMatch'

@Component
export default class MatchFavoriteButton extends Vue {
    @Prop({required: true})
    matchUuid!: string

    isFavorite: boolean = false
    loading: boolean = false
    showHideError: boolean = false

    showHideReason: boolean = false
    reason: string = ''

    get canFavorite(): boolean {
        return !!this.$store.state.currentUser
    }

    startFavoriteFlow() {
        if (this.isFavorite) {
            this.toggleFavorite()
        } else {
            this.showHideReason = true
        }
    }

    toggleFavorite() {
        this.loading = true

        let promise
        if (this.isFavorite) {
            promise = apiClient.removeMatchFavorite(this.matchUuid)
        } else {
            promise = apiClient.addMatchFavorite(this.matchUuid, this.reason)
        }

        promise.then(() => {
            this.isFavorite = !this.isFavorite
            this.showHideReason = false
        }).catch((err: any) => {
            console.log('Failed to toggle match favorite: ', err)
            this.showHideError = true
        }).finally(() => {
            this.loading = false
        })
    }

    @Watch('matchUuid')
    resyncStatus() {
        this.loading = true
        apiClient.isMatchFavorite(this.matchUuid).then((resp: ApiData<MatchFavoriteResponse>) => {
            this.isFavorite = resp.data.favorite
            this.reason = !!resp.data.reason ? resp.data.reason : ''
        }).catch((err: any) => {
            console.log('Failed to check if match is favorite: ', err)
        }).finally(() => {
            this.loading = false
        })
    }

    mounted() {
        this.resyncStatus()
    }
}

</script>