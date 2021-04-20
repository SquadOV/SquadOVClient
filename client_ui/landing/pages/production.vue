<template>
    <div id="signup" class="black--text">
        <div id="content">
            <v-row>
                <v-col cols="6">
                    <div class="text-h5 font-weight-bold">
                        Esports is growing, and the demand for content continues to grow.
                    </div>

                    <div class="text-h6 mt-4">
                        At the center of this growth is the demand for VODs and clips of the best moments of every tournament.
                        SquadOV helps by

                        <ul>
                            <li>
                                <b>Saving you time.</b>
                                SquadOV records games in separate VODs and lets you easily find the best moments using match details.
                            </li>

                            <li>
                                <b>Reducing complexity.</b>
                                A single platform to manage all your automatically captured VODs.
                            </li>

                            <li>
                                <b>Driving more engagement with your viewers and sponsors.</b>
                                Our smart technology fueled by gamers across a wide spectrum of games enables the automatic augmentation of video content.
                            </li>
                        </ul>
                    </div>

                    <div class="text-h5 mt-4">
                        Interested? Let's talk about how SquadOV can make help your next esports production.
                    </div>
                </v-col>

                <v-col cols="6">
                    <div class="d-flex align-center mt-4" style="height: 100%;">
                        <div id="form" style="width: 100%;"></div>
                    </div>
                </v-col>
            </v-row>
        </div>

        <div id="bg">
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class ProductionPage extends Vue {
    mounted() {
        Vue.nextTick(() => {
            //@ts-ignore
            hbspt.forms.create({
                region: 'na1',
                portalId: '7601735',
                formId: '11ba2023-9175-47ae-a674-d0644b705b6f',
                target: '#form',
            });
        })
    }
}
</script>

<style scoped>

#signup {
    height: calc(100vh - 48px);
    width: 100%;
}

#content {
    position: absolute;
    z-index: 5;
    top: 50%;
    left: 50%;
    transform: translateX(-50%) translateY(-50%);
    background: rgba(255, 255, 255, 0.7);
    border-radius: 4px;
    padding: 16px;
}

#bg {
    position: absolute;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background-image: url("/assets/splash/production.jpg");
    background-size: cover;
    background-position: center center;
    filter: blur(10px);
    z-index: 1;
}

</style>