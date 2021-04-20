<template>
    <div id="signup" class="black--text">
        <div id="content">
            <v-row>
                <v-col cols="6">
                    <div class="text-h5 font-weight-bold">
                        Whether you're a coach for a professional team or for amateurs, one thing remains the same: the need to review VODs.
                    </div>

                    <div class="text-h6 mt-4">
                        SquadOV can takes your coaching to the next level by

                        <ul>
                            <li>
                                <b>Saving you time.</b>
                                Spend your time coaching, not trying to find the right moment in a 6 hour VOD.
                            </li>

                            <li>
                                <b>Giving your insights more weight.</b>
                                Present your judgement using VODs synced with match details.
                            </li>

                            <li>
                                <b>Enabling your team to win more</b>.
                                VOD reviews are crucial to success and SquadOV makes them better.
                            </li>
                        </ul>
                    </div>

                    <div class="text-h5 mt-4">
                        Interested? Let's talk about how SquadOV can enhance your coaching.
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
export default class CoachPage extends Vue {
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
    background-image: url("/assets/splash/coaching.jpg");
    background-size: cover;
    background-position: center center;
    filter: blur(10px);
    z-index: 1;
}

</style>