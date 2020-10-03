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
                            <valorant-account-chooser
                                v-model="selectedAccount"
                                :options.sync="allAccounts"
                                class="mr-4"
                            >
                            </valorant-account-chooser>
<!--
                            <router-link :to="gamesTo">
                                <v-btn text class="mr-2" :disabled="!hasSelectedAccount">
                                    Games
                                </v-btn>
                            </router-link>

                            <router-link :to="agentsTo"> 
                                <v-btn text class="mr-2" :disabled="!hasSelectedAccount">
                                    Agents
                                </v-btn>
                            </router-link>

                            <router-link :to="mapsTo">
                                <v-btn text class="mr-2" :disabled="!hasSelectedAccount">
                                    Maps
                                </v-btn>
                            </router-link>

                            <router-link :to="weaponsTo">
                                <v-btn text :disabled="!hasSelectedAccount">
                                    Weapons
                                </v-btn>
                            </router-link>
-->

                            <v-btn icon @click="refreshAccounts">
                                <v-icon>
                                    mdi-refresh
                                </v-icon>
                            </v-btn>
                        </div>
                    </v-col>
                </v-row>

                <router-view v-if="hasSelectedAccount"></router-view>
                <v-row  justify="center" align="center" v-else>
                    <v-col cols="8">
                        <h2 class="text-center">Click the plus button to add a Valorant account, then start playing games of Valorant and SquadOV will automatically sync your VODs and match history.</h2>
                    </v-col>
                </v-row>
            </div>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch } from 'vue-property-decorator'
import { apiClient, ApiData } from '@client/js/api'
import { ValorantAccountData } from '@client/js/valorant/valorant_account'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import ValorantAccountChooser from '@client/vue/utility/valorant/ValorantAccountChooser.vue'
import * as pi from '@client/js/pages'

@Component({
    components: {
        LoadingContainer,
        ValorantAccountChooser
    }
})
export default class ValorantLogContainer extends Vue {
    allAccounts : ValorantAccountData[] | null = null
    selectedAccount : ValorantAccountData | null | undefined = null

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

    get hasSelectedAccount(): boolean {
        return !!this.selectedAccount;
    }

    @Watch('selectedAccount')
    @Watch('$route.params.account')
    onSelectAccount() {
        if (!this.selectedAccount) {
            return;
        }

        if (this.selectedAccount.puuid === this.$route.params.account) {
            return;
        }

        const params = {
            name: this.$route!.name!,
            params: {
                account: this.selectedAccount.puuid
            },
        }

        if (!this.$route.params.account) {
            this.$router.replace(params)
        } else {
            this.$router.push(params)
        }
       
    }

    @Watch('allAccounts')
    @Watch('$route.params.account')
    onRefreshAllAccounts() {
        if (!this.allAccounts || this.allAccounts.length === 0) {
            this.selectedAccount = null;
            return;
        }

        if (!!this.$route.params.account) {
            this.selectedAccount = this.allAccounts.find((ele : ValorantAccountData) => {
                return ele.puuid === this.$route.params.account
            })
        } else {
            this.selectedAccount = this.allAccounts[0]
        }

    }

    refreshAccounts() {
        apiClient.listValorantAccounts().then((resp : ApiData<ValorantAccountData[]>) => {
            this.allAccounts = resp.data
        }).catch((err : any) => {
            console.log('Failed to list valorant accounts: ' + err);
        })
    }

    mounted() {
        this.refreshAccounts()
    }
}

</script>

<style scoped>

</style>