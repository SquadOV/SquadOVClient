<template>
    <v-container fluid class="full-parent-height">
        <v-row class="full-parent-height flex-column" align-content="center" justify="center">
            <h1 class="text-center mb-4">Welcome back!</h1>

            <div class="d-flex">
                <router-link :to="logsTo">
                    <v-card class="choice" color="primary">
                        <v-card-title>
                            Game Logs
                        </v-card-title>
                    </v-card>
                </router-link>

                <router-link :to="performanceTo" >
                    <v-card class="choice" color="primary">
                        <v-card-title>
                            Performance
                        </v-card-title>
                    </v-card>
                </router-link>
            </div>
        </v-row>
    </v-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import * as pi from '@client/js/pages'

@Component
export default class Dashboard extends Vue {

    get logsTo() : any {
        return {
            name: pi.LogPageId
        }
    }

    get performanceTo() : any {
        return {
            name: pi.PerformancePageId
        }
    }

}

</script>

<style scoped>

.choice {
    margin-left: 8px;
    margin-right: 8px;
}

</style>