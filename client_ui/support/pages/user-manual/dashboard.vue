<template>
    <div>
        <h1>Dashboard</h1>
        <v-divider></v-divider>

        <div class="text-body-1">
            <div>
                <h2 id="overview">Overview</h2>

                The dashboard is your go-to screen to see what you and your friends have been playing. You can always return to the dashboard by clicking on the “SquadOV” button in the app navigation bar.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/dashboard/dashboard.jpg"
                    alt="SquadOV dashboard."
                    caption="Your hub for all things SquadOV."
                />

                <ol>
                    <li><NuxtLink to="#appnav">App Navigation</NuxtLink></li>
                    <li><NuxtLink to="#squads">Squad Management</NuxtLink></li>
                    <li><NuxtLink to="#recent">Recent Matches</NuxtLink></li>
                    <li><NuxtLink to="#playtime">Recorded Playtime</NuxtLink></li>
                    <li><NuxtLink to="#recording">Recording Status</NuxtLink></li>
                    <li><NuxtLink to="#news">Recent Updates</NuxtLink></li>
                    <li><NuxtLink to="#bugs">Bug Report</NuxtLink></li>
                </ol>
            </div>

            <div class="mt-4">
                <h2 id="appnav">App Navigation</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/dashboard/app_nav.jpg"
                    alt="SquadOV navigation bar."
                    caption="The SquadOV navigation bar."
                />

                The navigation bar is used to allow you to easily navigate between screens on SquadOV.

                <ol>
                    <li><b>Navigation buttons.</b> Go to the previous and next page in your history (similar to the back and forward pages in your browser).</li>
                    <li><b>Dashboard.</b> Return to the dashboard.</li>
                    <li><b>Game Logs.</b> Where you can view all mathces from you and your squadmates.</li>
                    <li><b>Performance.</b> Visualize your stats.</li>
                    <li><b>Library.</b> Where you can view all clips from you and your squadmates.</li>
                    <li><b>User Profile.</b> View your user profile, squads, and settings.</li>
                </ol>
            </div>

            <div class="mt-4">
                <h2 id="squads">Squad Management</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/dashboard/dashboard_squads.jpg"
                    alt="Squad management from the dashboard."
                    caption="Basic squad management from the dashboard."
                />

                The squad section of the dashboard is meant to give you easy-to-access controls and information about all your squads.

                <ol>
                    <li><b>Squad Dropdown.</b> A dropdown with all the squads you are a member of. Each squad is suffixed by the username of the user who created the squad.</li>
                    <li><b>New.</b> Create a new squad. See <NuxtLink to="/user-manual/squads#create">Creating Squads.</NuxtLink></li>
                    <li><b>Manage.</b> Manage your squad. See <NuxtLink to="/user-manual/squads#manage">Managing Squads.</NuxtLink></li>
                    <li><b>Invite.</b> Invite your friends to the selected squad by username or email. See #Inviting Friends.</li>
                    <li><b>Squad List.</b> A list of squad members and their current status.
                        <ul>
                            <li><i>Grey circle.</i> Offline.</li>
                            <li><i>Green circle.</i> Online.</li>
                            <li><i>Game Icon.</i> In-Game in the specified game.</li>
                            <li><i>Flashing game icon.</i> Recording in the specified game.</li>
                        </ul>
                    </li>
                </ol>
            </div>

            <div class="mt-4">
                <h2 id="recent">Recent Matches</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/dashboard/recent_games.jpg"
                    alt="Recent matches on the dashboard."
                    caption="A summary of you and your squadmates' recent recorded matches."
                />

                The recent games section of the dashboard will display a list of all games you and your squadmates recorded using SquadOV.
                To the left of the match summary is a VOD preview that will automatically play (with no sound) once you hover over the match.
                Check the <NuxtLink to="/user-manual/games">games</NuxtLink> section for game-specific information for what’s displayed in the match summary.
            </div>

            <div class="mt-4">
                <h2 id="playtime">Recorded Playtime</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/dashboard/recorded_playtime.jpg"
                    alt="Recorded playtime."
                    caption="How many hours have you been recording across all the games we support."
                />

                This section of the dashboard displays the amount of time in each of SquadOV’s supported games that you have recorded over the past week.
                The numbers automatically refresh every 30 minutes but you can force an immediate refresh by clicking on the <img src="/assets/user-manual/dashboard/refresh_button.jpg"></img> button.
            </div>

            <div class="mt-4">
                <h2 id="recording">Recording Status</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/dashboard/recording_status.jpg"
                    alt="Current recording status."
                    caption="SquadOV recording status and settings."
                />

                The recording status button in the bottom left corner of the app expands so that you have easy access to control the quality of your VODs.

                <ol>
                    <li><b>Status.</b> SquadOV has 3 states:
                        <ul>
                            <li>
                                <i>Available.</i>
                                Not in any game that SquadOV supports.
                            </li>
                            <li>
                                <i>In-Game.</i>
                                A supported game is open but not in a “match.”
                            </li>
                            <li>
                                <i>Recording.</i>
                                SquadOV is currently recording a “match.”
                            </li>
                        </ul>
                    </li>
                    <li>
                        <b>Play/Pause Button.</b>
                        If you wish to temporarily stop recording your games, you can hit the “Pause” button so that your games do not record.
                        When you wish to record again, hit the play button to resume normal operations.
                    </li>
                    <li>
                        <b>Video Resolution.</b>
                        See <NuxtLink to="/user-manual/settings#video">Video Settings</NuxtLink>.
                    </li>
                    <li>
                        <b>Video FPS.</b>
                        See <NuxtLink to="/user-manual/settings#video">Video Settings</NuxtLink>.
                    </li>
                    <li>
                        <b>Audio Output Capture.</b>
                        See <NuxtLink to="/user-manual/settings#audio">Audio Settings</NuxtLink>.
                    </li>
                    <li>
                        <b>Audio Input Capture.</b>
                        See <NuxtLink to="/user-manual/settings#audio">Audio Settings</NuxtLink>.
                    </li>
                </ol>
            </div>

            <div class="mt-4">
                <h2 id="news">Recent Updates</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/dashboard/recent_updates.jpg"
                    alt="Recent updates."
                    caption="News and updates from yours truly."
                />

                Every time we have news to share with you you’ll see a new entry in the Recent Updates section on your dashboard.
                To date this has been how we have communicated patch notes but we will use this to share more general information in the future as well.
                You can click on each update to read more.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/dashboard/recent_update_news.jpg"
                    alt="Example SquadOV patch notes."
                    caption="More information than you'll ever want to know."
                />
            </div>

            <div class="mt-4">
                <h2 id="status">Status Updates</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/dashboard/status.jpg"
                    alt="Service status."
                    caption="Sometimes our servers go down, this is where we'll let you know."
                />

                There may at times be issues with any of our backend servers that may cause a degraded experience for you.
                In such cases, we’ll let you know via our status update widget on the dashboard that will be displayed above the Recent Updates.
                You can read more about the issue by clicking on the <img src="/assets/user-manual/dashboard/status_button.jpg"></img> button; alternatively, you can check the status of our backend at any time by going to <a href="https://squadov.statuspage.io/">https://squadov.statuspage.io/</a>.
            </div>

            <div class="mt-4">
                <h2 id="bugs">Bug Reports</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/dashboard/bug_report.jpg"
                    alt="Bug reporting form."
                    caption="Find a bug? Report it!"
                />

                If you ever encounter a bug, we want to hear about it so we can improve the SquadOV experience for all users.
                Please include as much information as possible!
                Generally what we’d like to know are:
                <ul>
                    <li>What were you doing at the time of the bug?</li>
                    <li>Why is this a bug? What would you have expected to happen instead.</li>
                    <li>If the bug is regarding a particular match/VOD, please share a link to the match itself.</li>
                    <li>If the bug is regarding a particular clip, please share a link to the clip itself.</li>
                    <li>General system specifications: operating system, CPU, GPU, amount of RAM.</li>
                </ul>

                You can also use this form to submit feedback and/or feature requests.
                Note that you may receive an email from a developer following up on your bug report/feature request.
            </div>
        </div>
    </div>
</template>

<script lang="ts">
import Vue from 'vue'
import Component from 'nuxt-class-component'
@Component
export default class Dashboard extends Vue {

}
</script>