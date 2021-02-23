<template>
    <div>
        <h1>Teamfight Tactics</h1>
        <v-divider></v-divider>

        <div class="text-body-1">
            <div>
                <h2 id="account">Account Setup</h2>

                See <NuxtLink to="/user-manual/games/valorant#account">Valorant Account Setup</NuxtLink>. After syncing, the account dropdown on the Teamfight Tactics match history page will show your summoner name(s).
                You can manage your Riot accounts from the Teamfight Tactics game log screen instead.
            </div>

            <div class="mt-4">
                <h2 id="history">Match History</h2>
                
                The Teamfight Tactics match history page shows all your recent matches stored on SquadOV’s servers for the selected summoner name.
                Note that the match will not be displayed until the match is finished so if you finish 8th in the game, you will not see the match in your match history until a winner has been declared.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/tft/tft_match_history.jpg"
                    alt="TFT match history."
                    caption="TFT match history."
                />

                <ol>
                    <li>
                        <b>Little Legend.</b>
                        The Little Legend you used when playing this game.
                    </li>
                    <li>
                        <b>Rank and Traits.</b>
                        Your final placement and the final traits you had. Hovering over the traits shows you the number of units in the trait you had.
                    </li>
                    <li>
                        <b>Details.</b>
                        Date/time played, what game mode (normal/ranked), set, and game length.
                    </li>
                    <li>
                        <b>Units.</b>
                        Your final board along with their star level and items.
                        Chosen units are marked with the chosen icon.
                        Hovering over the unit displays the unit’s traits and items (with names).
                    </li>
                    <li>
                        <b>VOD Indicator.</b>
                        Shown when the game has a recorded VOD.
                    </li>
                </ol>
            </div>

            <div class="mt-4">
                <h2 id="details">Match Details</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/tft/tft_match_details_1.jpg"
                    alt="TFT match details."
                    caption="TFT match details."
                />

                <ol>
                    <li>
                        <b>Match Summary.</b>
                        See <NuxtLink to="#history">Match History</NuxtLink>.
                    </li>
                    <li>
                        <b>Match Sharing Button.</b>
                        See <NuxtLink to="/user-manual/games/common#share">Match Sharing</NuxtLink>.
                    </li>
                    <li>
                        <b>Video Player.</b>
                        See <NuxtLink to="/user-manual/vod#player">Video Player</NuxtLink>.
                    </li>
                    <li>
                        <b>VOD Picker.</b>
                        See <NuxtLink to="/user-manual/vod#picker">VOD Picker</NuxtLink>.
                        Hovering the VOD icon displays the user’s summoner name.
                    </li>
                </ol>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/tft/tft_match_details_2.jpg"
                    alt="TFT match details."
                    caption="TFT match details."
                />

                Advanced match details for each player at the end of the game.
                The information displayed here is equivalent to the information displayed in the Match Summary with the addition of some additional stats (Last Round, Gold Left, Players Eliminated, and Player Damage).
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class GamesTft extends Vue {

}

</script>