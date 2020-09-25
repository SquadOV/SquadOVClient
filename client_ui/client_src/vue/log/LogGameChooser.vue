<template>
    <v-container fluid class="full-parent-height">
        <v-row class="full-parent-height flex-column" align-content="center" justify="center">
            <h1 class="text-center mb-4">Select A Game</h1>

            <div class="d-flex">
                <router-link :to="valorantTo">
                    <v-card class="choice" color="primary">
                        <v-card-title>
                            Valorant
                        </v-card-title>
                    </v-card>
                </router-link>

                <router-link :to="aimlabTo" >
                    <v-card class="choice" color="primary">
                        <v-card-title>
                            Aim Lab
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
export default class LogGameChooser extends Vue {
    get valorantTo() : any {
        return {
            name: pi.ValorantLogPageId
        }
    }

    get aimlabTo() : any {
        return {
            name: pi.AimlabLogPageId
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