<template>
    <div>
        <h1>Welcome to SquadOV!</h1>
        <v-divider></v-divider>

        <div class="text-body-1">
            <div>
                Thanks for installing SquadOV, we're happy to have you here!
                We created SquadOV because we wanted a better way of reviewing VODs and sharing clips with friends than streaming to Twitch using OBS.
                We couldn't be more excited to have you join us on this journey!
            </div>

            <div class="mt-4">
                In this user manual, you'll find information about <b>everything</b> that SquadOV has to offer ranging from setting up your account to comparing how your Gridshot Ultimate score has increased over the past 6 months compared to your Microflex Ultimate score.
                If you ever see something in the SquadOV desktop app that you can't find in this user manual, <NuxtLink to="/">contact us</NuxtLink> so we can improve it!
            </div>

            <div class="mt-4">
                Happy recording!
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class Welcome extends Vue {

}

</script>