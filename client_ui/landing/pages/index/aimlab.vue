<template>
    <v-row no-gutters justify="center">
        <v-col cols="9">
            <v-row class="my-4">
                <v-col align-self="center" cols="3">
                    <div class="d-flex flex-column ">
                        <span class="text-h4">Fully automated.</span>
                        <span class="text-subtitle-1">
                            Install SquadOV, login, play Aim Lab. That's all you need to do to record a VOD with SquadOV.
                            No more OBS, no setup required.
                        </span>
                    </div>
                </v-col>

                <v-col cols="9">
                    <div class="d-flex justify-center">
                        <v-img
                            class="highlight-image"
                            max-width="80%"
                            src="assets/aimlab_triple_1.png"
                            contain
                        >
                        </v-img>
                    </div>
                </v-col>
            </v-row>

            <v-row class="my-4">
                <v-col cols="8">
                    <div class="d-flex justify-center">
                        <v-img
                            class="highlight-image"
                            max-width="80%"
                            src="assets/AimlabMatchHistory.png"
                            contain
                        >
                        </v-img>
                    </div>
                </v-col>

                <v-col align-self="center" cols="4">
                    <div class="d-flex flex-column">
                        <span class="text-h4">Better with friends.</span>
                        <span class="text-subtitle-1">
                            Create squads with friends to check their Aim Lab progress.
                            See their 100,000+ Gridshot Ultimate run using SquadOV.
                        </span>
                    </div>
                </v-col>
            </v-row>

            <v-row class="my-4">   
                <v-col align-self="center" cols="4">
                    <div class="d-flex flex-column ">
                        <span class="text-h4">Analytics.</span>
                        <span class="text-subtitle-1">
                            Track your improvement in games using SquadOV's automatic performance visualization tools.
                            Plot your Aim Lab Gridshot Ultimate score over time to check your improvement!
                            Good bye Excel!
                        </span>
                    </div>
                </v-col>

                <v-col cols="8">
                    <div class="d-flex justify-center">
                        <v-img
                            class="highlight-image"
                            max-width="80%"
                            src="assets/aimlab_graph.png"
                            contain
                        >
                        </v-img>
                    </div>
                </v-col>
            </v-row>
        </v-col>
    </v-row>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class AimlabIndex extends Vue {
    head() : any {
        return {
            title: 'SquadOV - Automatic VOD Analysis for Aim Lab',
        }
    }
}

</script>