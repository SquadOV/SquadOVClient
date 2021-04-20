<template>
    <div class="full-width" id="csgo-signup">
        <div id="csgo-signup-div">
            <div class="text-h4 font-weight-bold text black--text">
                CS:GO is coming soon to SquadOV!
            </div>

            <div class="text-subtitle-1 black--text">
                Tell us your name and email address to be one of the first to know when you'll be able to record, analyze, and share your CS:GO VODs with SquadOV.
            </div>

            <div class="mt-4">
                <div id="csgo-form"></div>
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class CsgoIndex extends Vue {
    head() : any {
        return {
            title: 'SquadOV - Better VODs, better match details for CS:GO',
        }
    }

    mounted() {
        Vue.nextTick(() => {
            //@ts-ignore
            hbspt.forms.create({
                region: 'na1',
                portalId: '7601735',
                formId: 'f76b72b1-0cdc-4b20-88ab-fb92e00ded87',
                target: '#csgo-form',
            });
        })
    }
}

</script>

<style scoped>

#csgo-signup {
    height: 80vh;
    background-image: url("/assets/splash/csgo_signup.jpg");
    background-size: cover;
    background-position: center center;
}

#csgo-signup-div {
    display: inline-block;
    position: relative;
    top: 50%;
    left: 100%;
    transform: translateX(-100%) translateX(-60px) translateY(-50%);
    max-width: 40%;
    background: rgba(255, 255, 255, 0.4);
    border-radius: 4px;
    padding: 16px;
}

</style>