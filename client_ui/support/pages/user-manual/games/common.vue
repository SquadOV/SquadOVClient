<template>
    <div>
        <h1>Common</h1>
        <v-divider></v-divider>

        <div class="text-body-1">
            <div>
                <h2 id="log">Game Log</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/common/gamelog.jpg"
                    alt="The game log."
                    caption="How to navigate the game log."
                />

                <ol>
                    <li>Squad dropdown.</li>
                    <li>Game selection.</li>
                    <li>Squad member selection.</li>
                    <li>Refresh button.</li>
                </ol>

                The game log displays all the matches SquadOV has stored for a single player at a time for a single game.
                Use the squad dropdown and the squad member selection dropdown menus to select which user’s matches you wish to view.
                Note that you can only view matches that you have access to (your own matches and squad member matches).
                Use the game selection bar to switch between all of SquadOV’s selected games.
            </div>

            <div class="mt-4">
                <h2 id="share">Match Sharing</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/common/match_sharing.jpg"
                    alt="Share matches via a URL."
                    caption="Copy and paste the URL to share."
                />

                The match sharing dialog allows you to share your VODs and match details with your friends using a URL that anyone can copy and paste into their browser. 
                Click the green copy button to copy the URL to your clipboard.
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class GamesCommon extends Vue {

}

</script>