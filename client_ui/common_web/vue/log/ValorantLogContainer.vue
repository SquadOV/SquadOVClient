<template>
    <loading-container :is-loading="!allAccounts">
        <template v-slot:default="{ loading }">
            <div v-if="!loading">
                <v-row>
                    <v-col cols="10">
                        <!-- 
                            Header div that lets user select accounts and also do some easy stats navigation.
                        -->
                        <div class="d-flex align-center">
                            <riot-account-chooser
                                v-model="selectedAccount"
                                @on-account-change="refreshAccounts"
                                :options.sync="allAccounts"
                                class="mr-4"
                            >
                            </riot-account-chooser>

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
                            <span v-if="isLocalUser">
                                Click the plus button to add a Valorant account, then start playing games of Valorant and SquadOV will automatically sync your VODs and match history.
                            </span>

                            <span v-else>
                                No Valorant accounts found for the selected user.
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
import { apiClient, ApiData } from '@client/js/api'
import { RiotAccountData } from '@client/js/valorant/valorant_account'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import RiotAccountChooser from '@client/vue/utility/riot/RiotAccountChooser.vue'
import CommonComponent from '../CommonComponent'

@Component({
    components: {
        LoadingContainer,
        RiotAccountChooser
    }
})
export default class ValorantLogContainer extends mixins(CommonComponent) {
    @Prop({required: true})
    userId!: number

    @Prop({required: true})
    puuid!: string | undefined

    allAccounts : RiotAccountData[] | null = null
    selectedAccount : RiotAccountData | null | undefined = null

/*
    get gamesTo() : any {
        return {
            name: pi.ValorantLogPageId,
            params: this.$route.params,
        }
    }

    get agentsTo() : any {
        return {
            params: this.$route.params,
        }
    }

    get mapsTo() : any {
        return {
            params: this.$route.params,
        }
    }

    get weaponsTo() : any {
        return {
            params: this.$route.params,
        }
    }
*/

    get isLocalUser(): boolean {
/// #if DESKTOP        
        return this.userId === this.$store.state.currentUser.id
/// #else
        return false
/// #endif
    }

    get hasSelectedAccount(): boolean {
        return !!this.selectedAccount;
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
            this.selectedAccount = this.allAccounts.find((ele : RiotAccountData) => {
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
        apiClient.listRiotValorantAccounts(this.userId).then((resp : ApiData<RiotAccountData[]>) => {
            this.allAccounts = resp.data
        }).catch((err : any) => {
            console.error('Failed to list valorant accounts: ' + err);
        })
    }

    mounted() {
        this.refreshAccounts()
    }
}

</script>

<style scoped>

</style>