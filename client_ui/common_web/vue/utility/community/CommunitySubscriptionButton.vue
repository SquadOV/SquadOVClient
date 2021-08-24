<template>
    <div class="d-flex justify-center align-center">
        <div v-if="community.allowTwitchSub">
            <loading-container :is-loading="twitchAccount === null">
                <template v-slot:default="{ loading }">
                    <div v-if="!loading">
                        <template v-if="twitchAccount.length > 0">
                            <v-btn
                                color="purple"
                                small
                            >
                                <v-icon class="mr-2">
                                    mdi-twitch
                                </v-icon>
                                Subscribe on Twitch
                            </v-btn>

                            <v-btn
                                class="ml-2"
                                icon
                                small
                            >
                                <v-icon>
                                    mdi-refresh
                                </v-icon>
                            </v-btn>
                        </template>

                        <v-btn
                            :to="accountsTo"
                            small
                            color="primary"
                            v-else
                        >
                            Link your Twitch Account
                        </v-btn>
                    </div>
                </template>
            </loading-container>
        </div>

        <div v-else>
            Non-Twitch subscriptions are not yet supported.
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { SquadOvCommunity } from '@client/js/squadov/community'
import { User2UserSubscription } from '@client/js/squadov/subscription'
import { TwitchAccount } from '@client/js/squadov/accounts'
import { apiClient, ApiData } from '@client/js/api'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { SettingsPageId } from '@client/js/pages'

@Component({
    components: {
        LoadingContainer
    }
})
export default class CommunitySubscriptionButton extends Vue {
    @Prop({required: true})
    community!: SquadOvCommunity

    @Prop({required: true})
    sub!: User2UserSubscription[]

    twitchAccount: TwitchAccount[] | null = null

    @Watch('$store.state.currentUser')
    refreshData() {
        this.twitchAccount = null
        if (!this.$store.state.currentUser) {
            return
        }

        apiClient.getLinkedTwitchAcccount().then((resp: ApiData<TwitchAccount[]>) => {
            this.twitchAccount = resp.data
        }).catch((err: any) => {
            console.log('Failed to get linked Twitch account: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }

    get accountsTo(): any {
        return {
            name: SettingsPageId,
            query: {
                inputTab: 2,
            }
        }
    }
}

</script>