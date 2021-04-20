<template>
    <div id="signup" class="black--text">
        <div id="content">
            <v-row>
                <v-col cols="6">
                    <div class="text-h5 font-weight-bold">
                        Did you just stream for 6 hours?
                        Now it's time to find your best moments to edit and upload to YouTube!
                    </div>

                    <div class="text-h6 mt-4">
                        But we both know that this process sucks; searching 6 hours of video to find the best 15 minutes is tedious.
                        SquadOV helps by

                        <ul>
                            <li>
                                <b>Saving you time.</b>
                                SquadOV records your games in separate VODs and lets you easily find your best moments using match details.
                            </li>

                            <li>
                                <b>Making you more money.</b>
                                More content, more money.
                                You can also monetize your VODs on SquadOV. 
                            </li>

                            <li>
                                <b>Driving more engagement with your viewers and sponsors.</b>
                                Our smart technology fueled by gamers across a wide spectrum of games enables the automatic augmentation of video content.
                            </li>
                        </ul>
                    </div>

                    <div class="text-h5 mt-4">
                        Interested? Let's talk about how SquadOV can enhance your content creation game.
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
export default class ContentCreatorPage extends Vue {
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
    background-image: url("/assets/splash/streaming.jpg");
    background-size: cover;
    background-position: center center;
    filter: blur(10px);
    z-index: 1;
}

</style>