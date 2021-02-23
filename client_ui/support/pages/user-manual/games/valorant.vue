<template>
    <div>
        <h1>Valorant</h1>
        <v-divider></v-divider>

        <div class="text-body-1">
            <div>
                <h2 id="account">Account Setup</h2>

                Before matches can be recorded in SquadOV, you must first connect your Riot account to SquadOV.
                To do this, open up SquadOV and navigate to the Valorant screen (1).

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/valorant/valorant_nav.jpg"
                    alt="Getting to the Valorant screen."
                    caption="How to get to the Valorant game logs."
                />

                Then hit the <v-icon color="success">mdi-plus</v-icon> button (2) to connect a new Riot account to your SquadOV account.
                This will open up a Riot games login page. 

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/valorant/rso_login.jpg"
                    alt="RSO Login."
                    caption="RSO Login."
                    width="25%"
                />

                Login with your Riot credentials (3) and authorize SquadOV to view your Account ID and have Offline Access (4). 

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/valorant/rso_auth.jpg"
                    alt="RSO Authorization."
                    caption="RSO Authorization."
                    width="25%"
                />

                After a successful login, SquadOV will sync with your Riot account (5). 

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/valorant/successful_rso.jpg"
                    alt="Success."
                    caption="RSO success."
                />

                You will know that the connection was successful if you can now see your Riot ID in your valorant screen (6).

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/valorant/account_list.jpg"
                    alt="Valorant account list."
                    caption="Valorant account list."
                />

                There are also additional controls to help you manage your linked Riot accounts.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/valorant/account_buttons.jpg"
                    alt="Additional Riot account controls."
                    caption="Additional Riot account controls."
                />

                <ol>
                    <li>
                        <b>Delete.</b>
                        Delete this Riot account from SquadOV's servers.
                        You will no longer be able to record matches for this account.
                    </li>
                    <li>
                        <b>Re-Sync.</b>
                        If you've changed your game name or tagline, you can use this button to manually resync that information with our servers.
                    </li>
                    <li>
                        <b>Refresh.</b>
                        Refresh the list of accounts.
                        Useful for when you just added the account and want to see the account in your account list immediately.
                    </li>
                </ol>
            </div>

            <div class="mt-4">
                <h2 id="history">Match History</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/valorant/valorant_nav.jpg"
                    alt="Getting to the Valorant screen."
                    caption="How to get to the Valorant game logs."
                />

                To browse your Valorant match history, click on the "Game" drop down menu and click on Valorant (1).

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/valorant/valorant_nav.jpg"
                    alt="Getting to the Valorant screen."
                    caption="How to get to the Valorant game logs."
                />

                This will bring you to the Valorant Match History screen.
                Clicking on the Riot ID drop down menu will allow you to navigate between your Riot IDs (2).
                Clicking on a match entry will navigate you to the respective match VOD and match details (3).

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/valorant/match_history.jpg"
                    alt="Valorant match history."
                    caption="Valorant match history."
                />

                Each match summary entry displays information about your performance in the match.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/valorant/match_summary.jpg"
                    alt="Valorant match summary."
                    caption="Valorant match summary."
                />

                <ol>
                    <li>
                        <b>Agent.</b>
                        What agent you played.
                        The left border is green for games you won and red for games you lost.
                    </li>

                    <li>
                        <b>Round Score.</b>
                        The final round score at the end of the game.
                        Text is green for games you won and red for games you lost.
                    </li>

                    <li>
                        <b>Combat Score Rank.</b>
                        Your combat score rank out of all players in the game.
                    </li>

                    <li>
                        <b>Accuracy.</b>
                        Your accuracy when hitting opponents throughout the game.
                        This includes all weapons as Riot's API is not precise enough for us to separate hits based on the weapon used.
                    </li>

                    <li>
                        <b>Personal Stats.</b>
                        Your KDA, damage per round (DPR), and combat score per round (CSPR).
                    </li>

                    <li>
                        <b>Game Details.</b>
                        Game mode and map.
                    </li>

                    <li>
                        <b>VOD Indicator.</b>
                        Indicator for whether or not you recorded a VOD for this match.
                    </li>
                </ol>
            </div>

            <div class="mt-4">
                <h2 id="details">Match Details</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/valorant/match_details_1.jpg"
                    alt="Valorant match details."
                    caption="Valorant match details."
                />

                After clicking on a match in your history, you will be able to watch your match VOD in its entirety.
                On the right side of the VOD (1) you can view all the kills and events (bomb plant and bomb defuse) that occurred during the round.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/valorant/full_match_details.jpg"
                    alt="Valorant match detail UI elements."
                    caption="Valorant match detail UI elements."
                />

                <ol>
                    <li>
                        <b>Match Summary.</b>
                        When you played the match, game mode, and map.
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
                    </li>
                    <li>
                        <b>Round Shop.</b>
                        Goes to the buy phase at the beginning of the round.
                    </li>
                    <li>
                        <b>Round Start.</b>
                        Goes to the start of the combat phase in the round.
                    </li>
                    <li>
                        <b>Event.</b>
                        Goes to the timestamp of the event in the VOD.
                    </li>
                </ol>

                To navigate to a specific round of the Valorant match, click on the round in the round timeline (2).
                Stats for each round are tracked (3) and the aggregate scoreboard is also logged (4).
                Summary stats, weapon usage, and head-to-head match up data is also kept (5).

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/valorant/match_details_2.jpg"
                    alt="Additional Valorant match details."
                    caption="Additional Valorant match details."
                />

                <div class="mt-4">
                    <h3 id="round">Round Summary</h3>

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/valorant/round_summary.jpg"
                        alt="Valorant round summary."
                        caption="Valorant round summary."
                    />

                    The round summary section displays each player's stats for the selected round.

                    <ol>
                        <li>
                            <b>Agent.</b>
                            The agent the stats are being displayed for.
                        </li>
                        <li>
                            <b>KDA.</b>
                            Kills / deaths / assists for this round only.
                        </li>
                        <li>
                            <b>Score.</b>
                            Combat score for this round only.
                        </li>
                        <li>
                            <b>Damage.</b>
                            Damage dealt in this round only.
                        </li>
                        <li>
                            <b>Inventory.</b>
                            The gun and armor loadout the player had at the beginning of the round.
                            This will not show any weapons picked up in the middle of the round.
                        </li>
                    </ol>    
                </div>

                <div class="mt-4">
                    <h3 id="scoreboard">Scoreboard</h3>

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/valorant/scoreboard.jpg"
                        alt="Valorant scoreboard."
                        caption="Valorant scoreboard."
                    />

                    The scoreboard displays each player's stats for the entirety of the game.
                    You can click on a different players in the scoreboard to change the displayed player summary.

                    <ol>
                        <li>
                            <b>Competitive Rank.</b>
                            The rank the player was at the start of the match.
                        </li>
                        <li>
                            <b>Agent.</b>
                            The agent the player chose.
                        </li>
                        <li>
                            <b>KDA.</b>
                            Kills / deaths / assists.
                        </li>
                        <li>
                            <b>CSPR.</b>
                            Combat score per round.
                            This is the total amount of combat score divided by the number of rounds played.
                        </li>
                        <li>
                            <b>DPR.</b>
                            Damage per round.
                            This is the total amount of damage done by the player divided by the number of rounds played.
                        </li>
                    </ol>

                    You can click on any of the headers (rank, player, KDA, CSPR, DPR) to sort by that metric instead.
                </div>

                <div class="mt-4">
                    <h3 id="summary">Player Summary</h3>

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/valorant/player_summary.jpg"
                        alt="Valorant player summary."
                        caption="Valorant player summary."
                    />

                    This displays a summary of stats for the player selected.
                    The stats displayed are:

                    <ul>
                        <li>
                            <b>KDA.</b>
                            Kills and assists divided by number of deaths.
                        </li>
                        <li>
                            <b>DPR.</b>
                            Average damage per round.
                        </li>
                        <li>
                            <b>HS%.</b>
                            Headshot percentage.
                        </li>
                        <li>
                            <b>CSPR.</b>
                            Average combat score per round.
                        </li>
                        <li>
                            <b>FB.</b>
                            Number of first bloods.
                        </li>
                        <li>
                            <b>FTD.</b>
                            Number of rounds where you were the first to die.
                        </li>
                        <li>
                            <b>DK.</b>
                            Number of rounds where you had a double kill.
                        </li>
                        <li>
                            <b>TK.</b>
                            Number of rounds where you had a triple kill.
                        </li>
                        <li>
                            <b>QK.</b>
                            Number of rounds where you had a quadra kill.
                        </li>
                        <li>
                            <b>PK.</b>
                            Number of rounds where you had a penta kill.
                        </li>
                    </ul>

                    The accuracy display is the same as the one in the match summary.
                </div>

                <div class="mt-4">
                    <h3 id="weapons">Weapons</h3>

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/valorant/weapons.jpg"
                        alt="Valorant player weapon usage."
                        caption="Valorant player weapon usage."
                    />

                    The weapon usage panel displays the weapons and abilities the player used to obtain kills.
                </div>

                <div class="mt-4">
                    <h3 id="h2h">Head-to-Head</h3>

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/valorant/h2h.jpg"
                        alt="Valorant head-to-head stats."
                        caption="Valorant head-to-head stats."
                    />

                    The H2H panel display the kills and deaths the selected player had against the selected opponent on the enemy team.
                    Each panel is expandable to display the round and round time the kill/death happened as well as the weapon/ability used.
                    You can click on the player button to jump to that event in the VOD.
                </div>
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class GamesValorant extends Vue {

}

</script>