<template>
    <loading-container :is-loading="!profileUser">
        <template v-slot:default="{ loading }">
            <div v-if="!loading">
                <v-container fluid>
                    <v-row class="justify-center">
                        <v-col cols="8">
                            <span class="text-h5 text--white font-weight-bold">
                                User Profile
                            </span>
                        </v-col>
                    </v-row>

                    <v-divider class="my-4"></v-divider>

                    <v-row class="justify-center">
                        <v-col cols="4">
                            <div class="font-weight-bold">
                                Username
                            </div>
                            
                            <div>
                                {{ profileUser.username }}
                            </div>
                        </v-col>

                        <v-col cols="4" v-if="isLocal">
                            <div class="font-weight-bold">
                                Email
                            </div>

                            <div>
                                {{ profileUser.email }}
                                <v-icon
                                    class="ml-2"
                                    color="success"
                                    small
                                    v-if="profileUser.verified"
                                >
                                    mdi-check-circle
                                </v-icon>
                            </div>
                        </v-col>
                    </v-row>
                </v-container>
            </div>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { SquadOVUser, getSquadOVUser } from '@client/js/squadov/user'
import { ApiData } from '@client/js/api' 
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'

@Component({
    components: {
        LoadingContainer
    }
})
export default class UserProfile extends Vue {
    @Prop({required: true})
    userId!: number

    profileUser: SquadOVUser | null = null

    get localUserId(): number {
        return this.$store.state.currentUser!.id
    }

    get isLocal(): boolean {
        return this.localUserId == this.profileUser?.id
    }

    @Watch('userId')
    refreshData() {
        this.profileUser = null
        getSquadOVUser(this.userId).then((resp: ApiData<SquadOVUser>) => {
            this.profileUser = resp.data
        }).catch((err: any) => {
            console.log('Failed to obtain user profile: ', err)
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>