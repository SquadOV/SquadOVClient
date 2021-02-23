<template>
    <div>
        <h1>League of Legends</h1>
        <v-divider></v-divider>

        <div class="text-body-1">
            <div>
                <h2 id="account">Account Setup</h2>

                See <NuxtLink to="/user-manual/games/valorant#account">Valorant Account Setup</NuxtLink>. After syncing, the account dropdown on the League of Legends match history page will show your summoner name(s).
                You can manage your Riot accounts from the League of Legends game log screen instead.
            </div>

            <div class="mt-4">
                <h2 id="history">Match History</h2>

                The League of Legends match history page shows all your recent matches stored on SquadOV’s servers for the selected summoner name.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/lol/lol_match_history.jpg"
                    alt="League of Legends match history."
                    caption="League of Legends match history."
                />

                <ol>
                    <li>
                        <b>Champion.</b>
                        The champion you played in this match.
                    </li>
                    <li>
                        <b>Details.</b>
                        Match details: game mode, game length, and when you played this game.
                    </li>
                    <li>
                        <b>KDA.</b>
                        Kills / deaths / assists (KDA).
                    </li>
                    <li>
                        <b>Stat Summary.</b>
                        Damage dealt to champions, number of minions killed, wards placed.
                    </li>
                    <li>
                        <b>Matchup.</b>
                        Every champion in the game, separated by teams.
                        Champions with a green border are on your team.
                        Champions with a red border are on the enemy team.
                        Champions with a yellow border are played by you.
                        Hovering over the champion icon displays the summoner name, KDA, and stat summary for that player.
                    </li>
                    <li>
                        <b>VOD Indicator.</b>
                        Displays when a recorded VOD is available.
                    </li>
                </ol>
            </div>

            <div class="mt-4">
                <h2 id="details">Match Details</h2>

                The League of Legends match details will display all the information we have about the match that we obtained from the official Riot Games League of Legends API along with the recorded VOD.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/lol/lol_match_details_1.jpg"
                    alt="League of Legends match details."
                    caption="League of Legends match details."
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
                    <li>
                        <b>Events.</b>
                        All the notable events that happened in the match.
                        The displayed events can be changed using the Event Filter.
                        Clicking the play button seeks the VOD to when the event happened.
                        Events with a green left border are events that benefit your team.
                        Events with a red left border are events that benefit the enemy team.
                        Events with a yellow left border are events that you participated in.
                    </li>
                    <li>
                        <b>Event Filter.</b>
                        Select which events (kills, wards, towers, inhibitors, dragons, barons, rift herald) to display in the events list.
                    </li>
                </ol>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/lol/lol_match_details_2.jpg"
                    alt="Additional League of Legends match details."
                    caption="Additional League of Legends match details."
                />

                <ol>
                    <li>
                        <b>Alternative Match Timeline.</b>
                        A timeline of the match that also displays all events shown in the event list.
                        Clicking on a timepoint moves to that timestamp in the VOD.
                    </li>
                    <li>
                        <b>Match Details.</b>
                        Displays advanced match details.
                    </li>
                </ol>

                There are three tabs for match details: scoreboard, advanced, and timeline that each display certain information about the match.

                <div class="mt-4">
                    <h3 id="scoreboard">Scoreboard</h3>

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/lol/lol_match_scoreboard.jpg"
                        alt="League of Legends scoreboard."
                        caption="The League of Legend scoreboard."
                    />

                    <ol>
                        <li>
                            <b>Team KDA.</b>
                            Total kills, deaths, and assists from all players on the same team.
                        </li>
                        <li>
                            <b>Team Economy.</b>
                            Total gold earned and gold spent by all players on the same team.
                        </li>
                        <li>
                            <b>Team Objectives.</b>
                            Total turrets, inhibitors, minions, barons, dragons, and rift heralds killed by all players on the same team.
                        </li>
                        <li>
                            <b>Player.</b>
                            Player champion, summoner name, and summoner spells.
                        </li>
                        <li>
                            <b>Player Items.</b>
                            Player items at the end of the match.
                        </li>
                        <li>
                            <b>Player KDA.</b>
                            KDA of the player at the end of the match.
                        </li>
                        <li>
                            <b>Player Economy.</b>
                            Gold earned and gold spent by the player at the end of the match.
                        </li>
                        <li>
                            <b>Minions Killed.</b>
                        </li>
                        <li>
                            <b>Wards Placed.</b>
                        </li>
                        <li>
                            <b>Damage to champions.</b>
                            Green: Damage dealt. Red: Damage received.
                        </li>
                    </ol>
                </div>

                <div class="mt-4">
                    <h3 id="advanced">Advanced Stats</h3>

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/lol/lol_advanced_stats.jpg"
                        alt="League of Legends advanced stats."
                        caption="The League of Legend advanced stats."
                    />

                    Advanced stats for each player grouped by category.
                    You can expand and collapse each group of stats by clicking the minus/plus button next to the group name.
                </div>

                <div class="mt-4">
                    <h3 id="timeline">Timeline</h3>

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/lol/lol_timeline.jpg"
                        alt="League of Legends stats timeline."
                        caption="The League of Legend stats timeline."
                    />

                    Visualization of available stats over the duration of the match. 

                    <ol>
                        <li>
                            <b>Subject.</b>
                            Which teams/players to display the stats for.
                        </li>
                        <li>
                            <b>Settings.</b>
                            Two settings are available:
                            <ul>
                                <li>
                                    <i>Team.</i>
                                    Whether to show aggregate team stats instead of individual player stats.
                                </li>
                                <li>
                                    <i>Diffs.</i>
                                    Whether to show the difference between the subject and the corresponding subject on the opposite team instead.
                                </li>
                            </ul>
                        </li>
                        <li>
                            <b>Stat Graph.</b>
                            See <NuxtLink to="/user-manual/stat-visualization#overview">Stat Visualization</NuxtLink>.
                        </li>
                        <li>
                            <b>Stat Selection.</b>
                            Available stats for League of Legends match details visualization.
                        </li>
                    </ol>
                </div>
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class GamesLol extends Vue {

}

</script>