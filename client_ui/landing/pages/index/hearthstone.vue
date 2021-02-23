<template>
    <v-row no-gutters justify="center">
        <v-col cols="9">
            <v-row class="my-4">
                <v-col align-self="center" cols="3">
                    <div class="d-flex flex-column ">
                        <span class="text-h4">Fully automated.</span>
                        <span class="text-subtitle-1">
                            Install SquadOV, login, play Hearthstone. That's all you need to do to record a VOD with SquadOV.
                            No more OBS, no setup required.
                        </span>
                    </div>
                </v-col>

                <v-col cols="9">
                    <div class="d-flex justify-center">
                        <v-img
                            class="highlight-image"
                            max-width="80%"
                            src="/assets/hearthstone_triple_1.jpg"
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
                    </v-carousel>
                </v-col>

                <v-col align-self="center" cols="4">
                    <div class="d-flex flex-column ">
                        <span class="text-h4">Automatic match details.</span>
                        <span class="text-subtitle-1">
                            Looking for when your Soul Demon Hunter deck 30-0'd the enemy Face Hunter?
                            Or looking for when your high-rolled dragons in Battlegrounds?
                            SquadOV automatically syncs your logs to make all that easy and more.
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
                            src="/assets/HearthstoneMatchHistory.jpg"
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
export default class HearthstoneIndex extends Vue {
    head() : any {
        return {
            title: 'SquadOV - Automatic VOD Analysis for Hearthstone',
        }
    }
}

</script>