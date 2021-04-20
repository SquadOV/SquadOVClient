<template>
    <v-row no-gutters justify="center">
        <v-col cols="9">
            <v-row class="my-4">
                <v-col align-self="center" cols="4">
                    <div class="d-flex flex-column ">
                        <span class="text-h4">Fully automated.</span>
                        <span class="text-subtitle-1">
                            Install, login, play a game. That's all you need to do to record a VOD with SquadOV.
                            No more OBS, no setup required.
                        </span>
                    </div>
                </v-col>

                <v-col cols="8">
                    <v-carousel cycle height="600">
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/valorant_triple_1.jpg"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/hearthstone_triple_1.jpg"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/aimlab_triple_1.jpg"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/wow_triple_1.jpg"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/lol_triple_1.jpg"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/tft_triple_1.jpg"
                            contain
                        >
                        </v-carousel-item>
                    </v-carousel>
                </v-col>
            </v-row>
            
            <v-row class="my-4">
                <v-col cols="8">
                    <v-carousel cycle height="600">
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/ValorantMatchDetails.jpg"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/HearthstoneMatchDetails.jpg"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/HearthstoneBGMatchDetails.jpg"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/WowMatchDetails_1.jpg"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/LeagueMatchDetails.jpg"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/TftMatchDetails.jpg"
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
                            src="/assets/ValorantMatchHistory.jpg"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/AimlabMatchHistory.jpg"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/HearthstoneMatchHistory.jpg"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/WowMatchHistory.jpg"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/LeagueMatchHistory.jpg"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/TftMatchHistory.jpg"
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
                            src="/assets/aimlab_graph.jpg"
                            contain
                        >
                        </v-carousel-item>
                        <v-carousel-item
                            class="carousel-image"
                            src="/assets/WowMatchDetails_2.jpg"
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