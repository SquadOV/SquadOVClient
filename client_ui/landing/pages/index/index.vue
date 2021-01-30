<template>
    <v-row no-gutters justify="center">
        <v-col cols="9">
            <v-row class="my-4">
                <v-col align-self="center" cols="3">
                    <div class="d-flex flex-column ">
                        <span class="text-h4">Fully automated.</span>
                        <span class="text-subtitle-1">
                            Install, login, play a game. That's all you need to do to record a VOD with SquadOV.
                            No more OBS, no setup required.
                        </span>
                    </div>
                </v-col>

                <v-col cols="9">
                    <div class="d-flex justify-center">
                        <v-img
                            class="highlight-image"
                            max-width="80%"
                            src="/assets/default_triple.png"
                            contain
                        >
                        </v-img>
                    </div>
                </v-col>
            </v-row>
            
            <v-row class="my-4">
                <v-col cols="8">
                    <v-carousel cycle height="600">
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/ValorantMatchDetails.png"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/HearthstoneMatchDetails.png"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/HearthstoneBGMatchDetails.png"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/WowMatchDetails_1.png"
                            contain
                        >
                        </v-carousel-item>
                    </v-carousel>
                </v-col>
                
                <v-col align-self="center" cols="4">
                    <div class="d-flex flex-column ">
                        <span class="text-h4">Automatic match details.</span>
                        <span class="text-subtitle-1">
                            Hate digging through hours of recorded footage to find your pentakill?
                            So do we.
                            SquadOV syncs your VOD to your game's match history so you can automatically jump to your favorite moment in your VOD.
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
                    <v-carousel cycle  height="600">
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/ValorantMatchHistory.png"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/AimlabMatchHistory.png"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/HearthstoneMatchHistory.png"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/WowMatchHistory.png"
                            contain
                        >
                        </v-carousel-item>
                    </v-carousel>
                </v-col>
            </v-row>

            <v-row class="my-4">   
                <v-col cols="8">
                    <v-carousel cycle  height="600">
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/aimlab_graph.png"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/WowMatchDetails_2.png"
                            contain
                        >
                        </v-carousel-item>
                    </v-carousel>
                </v-col>

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
            </v-row>
        </v-col>
    </v-row>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class DefaultIndex extends Vue {
}

</script>