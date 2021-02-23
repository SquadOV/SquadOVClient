<template>
    <div>
        <h1>Welcome to SquadOV!</h1>
        <v-divider></v-divider>

        <div class="text-body-1">
            <div>
                <h2 id="logs">Log Setup (Optional)</h2>

                Note that this step should be handled automatically by SquadOV; however, these are the instructions are for configuring Hearthstone manually if the need arises.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/hearthstone/logconfig.jpg"
                    alt="Log config folder."
                    caption="Log config folder."
                />
                Before Hearthstone games can be recorded in SquadOV, you must configure your log.config file to enable logging.
                To do this, type "%LOCALAPPDATA%/Blizzard/Hearthstone" (1) into your Windows Explorer.
                Then open your "log.config" file (2) and make sure it matches the following settings:

                <pre class="px-16 my-4">
[Power]
LogLevel=1
FilePrinting=true
ConsolePrinting=false
ScreenPrinting=false
Verbose=true
[Arena]
LogLevel=1
FilePrinting=true
ConsolePrinting=false
ScreenPrinting=false
Verbose=false
[PVPDR]
LogLevel=1
FilePrinting=true
ConsolePrinting=false
ScreenPrinting=false
Verbose=false
[Achievements]
LogLevel=1
Verbose=true
ScreenPrinting=false
FilePrinting=true
ConsolePrinting=false
[FullScreenFX]
LogLevel=1
ConsolePrinting=false
ScreenPrinting=false
FilePrinting=true
Verbose=true
[Decks]
ConsolePrinting=false
FilePrinting=true
ScreenPrinting=false
Verbose=false
LogLevel=1
                </pre>
            </div>

            <div class="mt-4">
                <h2 id="history">Match History</h2>

                <h3 id="history-overview">Overview</h3>

                To see your Constructed Hearthstone Match history, open up SquadOV, click on the "Game Logs" drop down and click "Hearthstone" (1).

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/hearthstone/hearthstone_nav.jpg"
                    alt="App navigation for Hearthstone game logs."
                    caption="App navigation for Hearthstone game logs."
                />

                Clicking on the Hearthstone entry shown in the above image will bring you to the Hearthstone "All Matches" tab.
                Hovering over a minion on the match history will display its card text (3). Click on the tabs, such as "All Matches" (4) will filter for the specific type of Hearthstone match you’re looking for.
                Clicking on a specific entry in your match history (5) will bring you to that VOD.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/games/hearthstone/hearthstone_match.jpg"
                    alt="Typical Hearthstone match summary."
                    caption="Typical Hearthstone match summary."
                />

                <div class="mt-4">
                    <h3 id="constructed">Constructed</h3>

                    To see your Constructed games match history after navigating to the Hearthstone section of SquadOV, click on the "Constructed" tab (1).
                    Clicking on the entries populated below the tabs will navigate you to the respective Constructed match VOD (2).

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/hearthstone/constructed.jpg"
                        alt="Constructed match history."
                        caption="Constructed match history."
                    />

                    From left to right, the match summary displays:

                    <ul>
                        <li>Your hero and deck name.</li>
                        <li>Details about the match (time played, mode, game length, final health).</li>
                        <li>Your final board state.</li>
                        <li>The enemy hero.</li>
                        <li>An indication of whether or not a VOD exists for this match.</li>
                    </ul>
                </div>

                <div class="mt-4">
                    <h3 id="arena">Arena</h3>

                    To see your Arena games match history after navigating to the Hearthstone section of SquadOV, click on the "Arena"" tab (1).
                    Each entry is the aggregate stats of each arena run. To see the individual games played within the arena run, click on the arena run entry (2).

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/hearthstone/arena_match_history.jpg"
                        alt="Arena match history."
                        caption="Arena match history."
                    />

                    After navigating to the arena run details, you can view the individual arena match VODs by clicking on each entry (1).
                    The cards drafted on the arena run are also tracked (2).

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/hearthstone/arena_run.jpg"
                        alt="Matches in an Arena run."
                        caption="Matches in an Arena run."
                    />
                </div>

                <div class="mt-4">
                    <h3 id="brawl">Brawl</h3>

                    To see your Tavern Brawl Hearthstone matches, click on the "Brawl" tab (1) after navigating to the Hearthstone Game Logs.
                    Clicking on the entries populated below the tabs will navigate you to the respective Tavern Brawl match VOD (2).

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/hearthstone/brawl_match_history.jpg"
                        alt="Brawl match history."
                        caption="Brawl match history."
                    />
                </div>

                <div class="mt-4">
                    <h3 id="battlegrounds">Battlegrounds</h3>

                    To see your Hearthstone Battleground matches, click on the "Battlegrounds" tab (1) after navigating to the Hearthstone Game Logs.
                    Clicking on the entries populated below the tabs will navigate you to the respective Battleground match VOD (2).

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/hearthstone/battlegrounds_match_history.jpg"
                        alt="Battlegrounds match history."
                        caption="Battlegrounds match history."
                    />

                    From left to right, the match summary displays:

                    <ul>
                        <li>Your selected hero.</li>
                        <li>Details about the match (time played, mode, game length, final health).</li>
                        <li>Your placement and rating.</li>
                        <li>Your final board state.</li>
                        <li>An indication of whether or not a VOD exists for this match.</li>
                    </ul>
                </div>

                <div class="mt-4">
                    <h3 id="duels">Duels</h3>

                    To see your Duels match history after navigating to the Hearthstone section of SquadOV, click on the "Duels" tab (1).
                    Each entry are the aggregate stats of each arena run. To see the individual games played within the Duels run, click on the Duels run entry (2).

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/hearthstone/duel_match_history.jpg"
                        alt="Duels match history."
                        caption="Duels match history."
                    />

                    After navigating to the arena run details, you can view the individual arena match VODs by clicking on the entries (3). The cards drafted on the arena run are also tracked (4). 

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/hearthstone/duel_run.jpg"
                        alt="Matches in a single duel run."
                        caption="Matches in a single duel run."
                    />
                </div>
            </div>

            <div class="mt-4">
                <h2 id="details">Match Details</h2>

                <div>
                    <h3 id="details-solo">1v1 Matches</h3>

                    After navigating to a Hearthstone 1v1 match (eg: Constructed, Arena, Brawl or Duels match), you will be able to see the match VOD and detailed information about your match.
                    Historical events such as playing a minion, playing a spell, hero powering, attacking, are all tracked on the right (1). 

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/hearthstone/solo_events.jpg"
                        alt="Hearthstone match events."
                        caption="Hearthstone match events."
                    />

                    You can also view available cards at a particular point in time of the VOD by clicking on the "Deck" tab on the right (2).

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/hearthstone/deck_time.jpg"
                        alt="ck state at the beginning of the selected round."
                        caption="Deck state at the beginning of the selected round."
                    />

                    Your deck and your opponent’s deck (cards you’ve seen in the match) are also tracked below (3). To skip to a particular turn, hit the turn number on the timeline below the VOD (4).

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/hearthstone/deck_final_timeline.jpg"
                        alt="Player decks."
                        caption="Player decks."
                    />
                </div>

                <div class="mt-4">
                    <h3 id="details-bg">Battlegrounds</h3>

                    After navigating to a Battleground match, you will be able to see your last warband (1).
                    Historical events such as buying a unit, rolling a new warband, placing a unit, or freezing will be displayed on the right (2).
                    Hitting the play button on the right of the event in the log will navigate the VOD to that point in the video.

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/hearthstone/bg_events.jpg"
                        alt="Battlegrounds match events."
                        caption="Battlegrounds match events."
                    />

                    To skip to a particular turn of the Battleground match, click on the bottom timeline.
                    Clicking on the shopping icon will navigate you to the buy phase of that turn, while the combat icon will navigate you to the battle phase of the respective turn (3).

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/hearthstone/bg_timeline.jpg"
                        alt="Battlegrounds match timeline."
                        caption="Battlegrounds match timeline."
                    />

                    You can also search through the available units for all the tiers in Battleground by clicking on the tavern tab (4) and the respective tier (5).

                    <GenericImage
                        class="my-4"
                        src="/assets/user-manual/games/hearthstone/bg_tavern.jpg"
                        alt="Battlegrounds tavern tech level overview."
                        caption="Battlegrounds tavern tech level overview."
                    />
                </div>
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class GamesHearthstone extends Vue {

}

</script>