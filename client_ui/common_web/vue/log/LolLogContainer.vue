<template>
    <loading-container :is-loading="!allAccounts">
        <template v-slot:default="{ loading }">
            <div v-if="!loading">
                <v-row>
                    <v-col cols="10">
                        <div class="d-flex align-center">
                            <summoner-account-chooser
                                v-model="selectedAccount"
                                :options.sync="allAccounts"
                                @on-account-change="refreshAccounts"
                                class="mr-4"
                            >
                            </summoner-account-chooser>

                            <v-btn icon @click="refreshAccounts">
                                <v-icon>
                                    mdi-refresh
                                </v-icon>
                            </v-btn>
                        </div>
                    </v-col>
                </v-row>

                <router-view v-if="hasSelectedAccount"></router-view>
                <v-row justify="center" align="center" v-else>
                    <v-col cols="8">
                        <h2 class="text-center">
                            <span>
                                No League of Legends accounts found for the selected user.
                            </span>
                        </h2>
                    </v-col>
                </v-row>
            </div>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Component, { mixins } from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { RiotSummoner } from '@client/js/riot/summoner'
import { apiClient, ApiData } from '@client/js/api'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import SummonerAccountChooser from '@client/vue/utility/riot/SummonerAccountChooser.vue'
import CommonComponent from '../CommonComponent'

@Component({
    components: {
        LoadingContainer,
        SummonerAccountChooser,
    }
})
export default class LolLogContainer extends mixins(CommonComponent) {
@Prop({required: true})
    userId!: number

    @Prop({required: true})
    puuid!: string | undefined

    allAccounts : RiotSummoner[] | null = null
    selectedAccount : RiotSummoner | null | undefined = null

    get hasSelectedAccount(): boolean {
        return !!this.selectedAccount
    }

    @Watch('selectedAccount')
    @Watch('puuid')
    onSelectAccount() {
        if (!this.selectedAccount) {
            return;
        }

        if (this.selectedAccount.puuid === this.puuid) {
            return;
        }

        const params : any = {
            name: this.$route!.name!,
            params: {
                userId: this.userId,
                account: this.selectedAccount.puuid
            },
            query: this.cleanQuery
        }

        if (!this.puuid) {
            this.$router.replace(params)
        } else {
            this.$router.push(params)
        }
    }

    @Watch('allAccounts')
    @Watch('puuid')
    onRefreshAllAccounts() {
        if (!this.allAccounts || this.allAccounts.length === 0) {
            this.selectedAccount = null;
            return;
        }

        if (!!this.puuid) {
            this.selectedAccount = this.allAccounts.find((ele : RiotSummoner) => {
                return ele.puuid === this.puuid
            })

            // We need this backup just in case we're switching from another player (in which case the puuid will be invalid).
            if (!this.selectedAccount) {
                this.selectedAccount = this.allAccounts[0]
            }
        } else {
            this.selectedAccount = this.allAccounts[0]
        }
    }
    
    @Watch('userId')
    refreshAccounts() {
        apiClient.listRiotLolAccounts(this.userId).then((resp : ApiData<RiotSummoner[]>) => {
            this.allAccounts = resp.data
        }).catch((err : any) => {
            console.error('Failed to list LoL accounts: ' + err);
        })
    }

    mounted() {
        this.refreshAccounts()
    }
}

</script>