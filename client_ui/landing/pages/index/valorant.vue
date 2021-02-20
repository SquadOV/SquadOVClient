<template>
    <v-row no-gutters justify="center">
        <v-col cols="9">
            <v-row class="my-4">
                <v-col align-self="center" cols="3">
                    <div class="d-flex flex-column ">
                        <span class="text-h4">Fully automated.</span>
                        <span class="text-subtitle-1">
                            Install SquadOV, login, play Valorant. That's all you need to do to record a VOD with SquadOV.
                            No more OBS, no setup required.
                        </span>
                    </div>
                </v-col>

                <v-col cols="9">
                    <div class="d-flex justify-center">
                        <v-img
                            class="highlight-image"
                            max-width="80%"
                            src="/assets/valorant_triple_1.png"
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
                            src="/assets/ValorantMatchDetails.png"
                            contain
                        >
                        </v-img>
                    </div>
                </v-col>

                <v-col align-self="center" cols="4">
                    <div class="d-flex flex-column ">
                        <span class="text-h4">Automatic match details.</span>
                        <span class="text-subtitle-1">
                            Looking for your Bucky pentakill? Or your 1v5 clutch on Ascent while playing Sova?
                            SquadOV automatically syncs your official Valorant match history details to your VOD to make looking for key moments easier.
                        </span>
                    </div>
                </v-col>
            </v-row>

            <v-row class="my-4">
                <v-col align-self="center" cols="4">
                    <div class="d-flex flex-column">
                        <span class="text-h4">Better with friends.</span>
                        <span class="text-subtitle-1">
                            Create squads with friends to share match history and VODs.
                            Easily relive your friends' best moments using SquadOV's automatic VOD recording and match history sync.
                            In the same game? Switch POVs with a single click!
                        </span>
                    </div>
                </v-col>

                <v-col cols="8">
                    <div class="d-flex justify-center">
                        <v-img
                            class="highlight-image"
                            max-width="80%"
                            src="/assets/ValorantMatchHistory.png"
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
export default class ValorantIndex extends Vue {
}

</script>