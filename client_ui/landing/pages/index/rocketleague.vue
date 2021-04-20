<template>
    <div class="full-width" id="signup">
        <div id="signup-div">
            <div class="text-h4 font-weight-bold text black--text">
                Rocket League is coming soon to SquadOV!
            </div>

            <div class="text-subtitle-1 black--text">
                Tell us your name and email address to be one of the first to know when you'll be able to record, analyze, and share your Rocket League VODs with SquadOV.
            </div>

            <div class="mt-4">
                <div id="form"></div>
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class RocketLeagueIndex extends Vue {
    head() : any {
        return {
            title: 'SquadOV - Better VODs, better match details for Rocket League',
        }
    }

    mounted() {
        Vue.nextTick(() => {
            //@ts-ignore
            hbspt.forms.create({
                region: 'na1',
                portalId: '7601735',
                formId: '48aeb533-5243-436c-a3c4-b1f945f959b8',
                target: '#form',
            });
        })
    }
}

</script>

<style scoped>

#signup {
    height: 80vh;
    background-image: url("/assets/splash/rocketleague_signup.jpg");
    background-size: cover;
    background-position: center center;
}

#signup-div {
    display: inline-block;
    position: relative;
    top: 50%;
    left: 100%;
    transform: translateX(-100%) translateX(-60px) translateY(-50%);
    max-width: 40%;
    background: rgba(255, 255, 255, 0.7);
    border-radius: 4px;
    padding: 16px;
}

</style>