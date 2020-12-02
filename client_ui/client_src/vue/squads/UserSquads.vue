<template>
    <loading-container :is-loading="!mySquads">
        <template v-slot:default="{ loading }">
            <div v-if="!loading">
                <div class="text-h5 font-weight-bold">
                    My Squads
                </div>

                <v-divider class="my-4"></v-divider>

                <v-tabs>
                    <v-tab>
                        Squads
                    </v-tab>

                    <v-tab-item>

                    </v-tab-item>

                    <v-tab>
                        Invites
                    </v-tab>

                    <v-tab-item>

                    </v-tab-item>
                </v-tabs>
            </div>
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { ApiData } from '@client/js/api' 
import { Squad } from '@client/js/squadov/squad'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'

@Component({
    components: {
        LoadingContainer
    }
})
export default class UserSquads extends Vue {
    @Prop({required: true})
    userId!: number

    mySquads: Squad[] | null = null

    @Watch('userId')
    refreshData() {
        this.mySquads = null
    }

    mounted() {
        this.refreshData()
    }
}

</script>