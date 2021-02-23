<template>
    <div>
        <h1>World of Warcraft</h1>
        <v-divider></v-divider>

        <div class="text-body-1">
            <div>
                <h2 id="combatlog">Combat Logs</h2>

                Before Raid Encounters, M+, or Arenas can be recorded in SquadOV, we need to make sure our combat logs are recording properly.
                We recommend you use the assistance of a combat log addon.
                For Arena, we recommend you use <a href="https://www.curseforge.com/wow/addons/arena-combat-logger">Arena Combat Logger</a>.
                For PvE content, we recommend you use <a href="https://www.curseforge.com/wow/addons/autolog">AutoLog</a>.
                Having multiple enabled combat logs will often cause no logs to be generated so be sure to only have one addon enabled at a given time (1).

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/wow/addon.jpg"
                    alt="WoW Addons."
                    caption="WoW Addons."
                />

                Next we need to go to our World of Warcraft menu > System > Network and make sure "Advanced Combat Logging" is turned on (4):

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/wow/advanced_combat.jpg"
                    alt="WoW Advanced Combat Logging."
                    caption="WoW Advanced Combat Logging."
                />
            </div>

            <div class="mt-4">
                <h2 id="history">Match History</h2>

                <div class="mt-4">
                    <h3 id="history-overview">Overview</h3>

                    To navigate to your World of Warcraft Match History, click on the Game Logs drop down menu and click on "World of Warcraft" (1).

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/wow/wow_nav.jpg"
                        alt="WoW Game Log Navigation."
                        caption="WoW Game Log Navigation."
                    />

                    All matches are organized by character.
                    To change characters click on the drop down menu and select the character you want to view the log for (2).

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/wow/char_select.jpg"
                        alt="WoW Character Selection."
                        caption="WoW Character Selection."
                    />
                </div>

                <div class="mt-4">
                    <h3 id="history-encounters">Encounters</h3>

                    The default page when you navigate to the World of Warcraft Game Logs section is encounters.
                    Clicking on an entry will navigate you to that boss pull’s VOD and combat details (3).
                    To navigate back to this tab, click on the "Encounters" tab (4).
                    You can also change characters within each individual tab.

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/wow/encounter.jpg"
                        alt="WoW Boss Encounters."
                        caption="WoW Boss Encounters."
                    />

                    From left to right, the displayed information in the match summary are:

                    <ul>
                        <li>A green highlight for a successful pull and red highlight for a failed pull.</li>
                        <li>The boss encounter, the instance, and the difficulty.</li>
                        <li>Time of the pull and duration of the enoucnter.</li>
                        <li>All characters who participated in the pull. The icon displays their class specialization. Hovering over the icon displays their name and ilvl.</li>
                    </ul>
                </div>

                <div class="mt-4">
                    <h3 id="history-keystones">Keystones</h3>

                    To navigate to Mythic+ Keystone VODs, click on the "Keystones" tab (5).
                    Clicking on an entry will navigate you to the full Mythic+ run’s VOD (6).

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/wow/keystone.jpg"
                        alt="WoW M+ Keystones."
                        caption="WoW M+ Keystones."
                    />

                    From left to right, the displayed information in the match summary are:

                    <ul>
                        <li>The M+ dungeon and the keystone level.</li>
                        <li>Time of the pull and duration of the enoucnter.</li>
                        <li>All characters who participated in the pull. The icon displays their class specialization. Hovering over the icon displays their name and ilvl.</li>
                    </ul>
                </div>

                <div class="mt-4">
                    <h3 id="history-arenas">Arena</h3>

                    To navigate to Arena VODs, click on the "Arena" tab (7).
                    Clicking on an entry will navigate you to the arena match VOD (8).

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/wow/arena.jpg"
                        alt="WoW Arena."
                        caption="WoW Arena."
                    />

                    From left to right, the displayed information in the match summary are:

                    <ul>
                        <li>The arena map, the mode, and the final rating of your team after the match.</li>
                        <li>Time of the pull and duration of the enoucnter.</li>
                        <li>
                            All characters who participated in the pull.
                            The icon displays their class specialization and is highlighted green if they are on your team and red if they are on the enemy team.
                            Hovering over the icon displays their name and ilvl.
                        </li>
                    </ul>
                </div>
            </div>

            <div class="mt-4">
                <h2 id="details">Match Details</h2>

                After navigating to a particular encounter, keystone or arena match, you will be able to view the VOD recording, see the composition of your group (1), navigate the timeline of important events such as bosses killed, player deaths, etc (2) and check out damage, healing, and damage received charts (3).

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/wow/wow_match_details.jpg"
                    alt="WoW Match Details."
                    caption="WoW Match Details."
                />

                To filter which events show up on the timeline (2), click on the filter settings (4) then customize the events you want to see in your timeline (5).

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/wow/event_filter.jpg"
                    alt="WoW Event Filter."
                    caption="WoW Event Filter."
                />

                The stat timeline will display the damage dealt, heals, and damage received per second statistics for everyone in the match.
                The stat value is averaged over 5 second intervals.
                Currently, only bloodlust type spell auras and player deaths are rendered on the graph.
                Clicking on a particular point in time in the chart will also navigate the VOD to that particular point in time (6).

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/wow/stat_timeline.jpg"
                    alt="WoW Stat Timeline."
                    caption="WoW Stat Timeline."
                />

                You can click on the gear button at the top right of the graph to modify some settings on what/how the stats are displayed.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/wow/stat_settings.jpg"
                    alt="WoW Stat Display Settings."
                    caption="WoW Stat Display Settings."
                />

                The available settings are:
                <ul>
                    <li>
                        <b>Separate graphs.</b>
                        Whether to render a single graph per player in the match or a single graph with multiple lines.
                    </li>
                    <li>
                        <b>Show time.</b>
                        Whether to render a yellow bar that indicates the current time in the VOD.
                    </li>
                    <li>
                        <b>Show events.</b>
                        Whether to display player death events in the timeline.
                    </li>
                    <li>
                        <b>Show bloodlust.</b>
                        Whether to show when bloodlust type auras are in effect.
                    </li>
                </ul>

                You can view more information about the stat visualization graph UI <NuxtLink to="/user-manual/stat-visualization#overview">here</NuxtLink>.
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class GamesWow extends Vue {

}

</script>