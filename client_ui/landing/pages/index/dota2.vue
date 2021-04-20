<template>
    <div class="full-width" id="signup">
        <div id="signup-div">
            <div class="text-h4 font-weight-bold text black--text">
                Dota 2 is coming soon to SquadOV!
            </div>

            <div class="text-subtitle-1 black--text">
                Tell us your name and email address to be one of the first to know when you'll be able to record, analyze, and share your Dota 2 VODs with SquadOV.
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
export default class Dota2Index extends Vue {
    head() : any {
        return {
            title: 'SquadOV - Better VODs, better match details for Dota 2',
        }
    }

    mounted() {
        Vue.nextTick(() => {
            //@ts-ignore
            hbspt.forms.create({
                region: 'na1',
                portalId: '7601735',
                formId: '35756d75-f157-4f50-9c20-294869fefb05',
                target: '#form',
            });
        })
    }
}

</script>

<style scoped>

#signup {
    height: 80vh;
    background-image: url("/assets/splash/dota2_signup.jpg");
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