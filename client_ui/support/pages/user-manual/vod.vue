<template>
    <div>
        <h1>VODs</h1>
        <v-divider></v-divider>

        <div class="text-body-1">
            <div>
                <h2 id="player">Video Player</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/vod/video_player.jpg"
                    alt="The SquadOV video player."
                    caption="The SquadOV video player."
                />

                The video player is used whenever SquadOV plays a controllable video. Thus, you will see this video player used on the match details pages as well as when viewing clips.

                <ol>
                    <li>Play/Pause.</li>
                    <li>Volume Control.</li>
                    <li>Video Timestamp.</li>
                    <li>Playback Speed Control.</li>
                    <li>Toggle Picture-in-Picture mode.</li>
                    <li>Toggle Theater Mode (if available).</li>
                    <li>Toggle Full-Screen Mode.</li>
                </ol>

                The video player also comes with a set of keyboard shortcuts that should be familiar to anyone who uses the YouTube player.
                <v-simple-table>
                    <template v-slot:default>
                        <thead>
                            <tr>
                                <th class="text-left">
                                    Key(s)
                                </th>
                                <th class="text-left">
                                    Action
                                </th>
                            </tr>
                        </thead>
                        <tbody>
                            <tr>
                                <td>Space Bar, "K"</td>
                                <td>Play/Pause.</td>
                            </tr>
                            <tr>
                                <td>"F"</td>
                                <td>Toggle fullscreen.</td>
                            </tr>
                            <tr>
                                <td>Arrow Left</td>
                                <td>Go back 5 seconds.</td>
                            </tr>
                            <tr>
                                <td>Arrow Right</td>
                                <td>Go forward 5 seconds.</td>
                            </tr>
                            <tr>
                                <td>"J"</td>
                                <td>Go back 10 seconds.</td>
                            </tr>
                            <tr>
                                <td>"L"</td>
                                <td>Go forward 10 seconds.</td>
                            </tr>
                            <tr>
                                <td>Arrow Up</td>
                                <td>Incrase volume by 5%.</td>
                            </tr>
                            <tr>
                                <td>Arrow Down</td>
                                <td>Decrease volume by 5%.</td>
                            </tr>
                            <tr>
                                <td>"M"</td>
                                <td>Mute/unmute video.</td>
                            </tr>
                            <tr>
                                <td>0-9</td>
                                <td>Go to the specified % in the video.
                                    <ul>
                                        <li>0: 0%</li>
                                        <li>1: 10%</li>
                                        <li>2: 20%</li>
                                        <li>Etc.</li>
                                    </ul>
                                </td>
                            </tr>
                            <tr>
                                <td>Home</td>
                                <td>Go to the beginning of the video (same as pressing 0).</td>
                            </tr>
                            <tr>
                                <td>End</td>
                                <td>Go to near the end of the video.</td>
                            </tr>
                            <tr>
                                <td>"I"</td>
                                <td>Toggle picture-in-picture mode.</td>
                            </tr>
                            <tr>
                                <td>"T"</td>
                                <td>Toggle theater mode.</td>
                            </tr>
                        </tbody>
                    </template>
                </v-simple-table>
            </div>

            <div class="mt-4">
                <h2 id="picker">VOD Picker</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/vod/vod_picker.jpg"
                    alt="The SquadOV video POV picker."
                    caption="The SquadOV video POV picker."
                />

                The VOD POV picker contains some information that is game-specific; however, the general layout is shared regardless of what game’s VOD is being played back.
                You will generally find the VOD POV picker right under the video player.

                <ol>
                    <li>
                        <b>Game-Specific Display of Available VODs to choose from.</b>
                        Click on an icon to change VOD POVs.
                    </li>
                    <li>
                        <b>Delete VOD.</b>
                        Permanently deletes the VOD from our servers. You may only delete VODs that you recorded.
                    </li>
                    <li>
                        <b>Download VOD.</b>
                        Download a copy of the VOD to a selected location on disk. You may only download VODs that you recorded.
                    </li>
                    <li>
                        <b>Clip VOD.</b>
                        Create a clip of the selected VOD in our clip editor. You may only clip VODs that you recorded.
                    </li>
                    <li>
                        <b>Clip Library.</b>
                        Opens the clip library for this match. View all the clips for this match that were clipped by you and your squadmates.
                    </li>
                </ol>
            </div>

            <div class="mt-4">
                <h2 id="clipeditor">Clip Editor</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/vod/clip_editor.jpg"
                    alt="The SquadOV clip editor."
                    caption="The SquadOV clip editor."
                />

                The clip editor is your interface for easily creating shareable clips from your recorded VODs.
                Note that there is a current limit of 45 seconds on all clips.

                <ol>
                    <li>
                        <b>Clip Video Player.</b>
                        The same video player as the one discussed in <NuxtLink to="/user-manual/vod#player">Video Player</NuxtLink>.
                    </li>
                    <li>
                        <b>VOD Timeline.</b>
                        Displays a timeline of the VOD and visually displays where you are in the VOD. Click on the timeline to navigate to that point in the VOD.
                    </li>
                    <li>
                        <b>Clip Timeline.</b>
                        Displays another timeline of the VOD and displays a green bar and red bar to mark the beginning and end of the clip respectively.
                        Drag the bar to adjust the clip location.
                    </li>
                    <li>
                        <b>Clip Settings.</b>
                        <ul>
                            <li>
                                <i>Sync Timestamp.</i>
                                Enable this if you want changes to the current timestamp of the VOD on the match details page to change the current timestamp of the VOD on the clip editor.
                                This can be useful as it allows you to use the event-based navigation options on the match details button to easily go to the desired location in the clip editor.
                            </li>
                            <li>
                                <i>Preview Clip.</i>
                                Enabling this forces the clip editor’s video player to only play the VOD within the timestamps specified by the start/stop time of the clip.
                            </li>
                        </ul>
                    </li>
                    <li>
                        <b>Current VOD Time.</b>
                        A textual display of the currently displayed VOD time.
                    </li>
                    <li>
                        <b>Clip Time.</b>
                        A textual display of the beginning and end of the clip.
                        You can manually type in the time to change the clip start/stop time.
                        You can either type the time as a total number of seconds or in the form of XX:YY:ZZ where XX is the number of hours, YY is the number of minutes, and ZZ is the number of seconds.
                        Alternatively, click the <img src="/assets/user-manual/vod/copy_time_button.jpg"></img> button to use the current VOD time as the clip start/stop time. 
                    </li>
                </ol>

                Once you’ve finalized the time range you wish to clip, click the “Clip it!” button to generate the final clip that will be stored on our servers.
                This may take a few seconds.
                Once the app is finished creating your clip, a dialog window will pop up. 

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/vod/create_clip.jpg"
                    alt="The create clip dialog."
                    caption="The create clip dialog - one last step before we save the clip!"
                />

                Give your clip a cool title and description and click ‘Save’ to upload the clip to SquadOV’s servers.
                Once the clip is done uploading, you’ll see a SquadOV share link that you can copy and paste to share the clip with your friends.

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/vod/clip_share.jpg"
                    alt="The clip sharing URL."
                    caption="Copy and paste to share!"
                />
            </div>

            <div class="mt-4">
                <h2 id="clipviewer">Clip Viewer</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/vod/clip_viewer.jpg"
                    alt="The clip viewing scren."
                    caption="Don't forget to like and subscribe!"
                />

                The clip viewer interface allows you to view, comment on, like, and share your clips.
                <ol>
                    <li>
                        <b>Video Player.</b>
                        See more in <NuxtLink to="/user-manual/vod#player">Video Player</NuxtLink>.
                    </li>
                    <li>
                        <b>Title.</b>
                        The title of the clip prefixed by the icon of the game being played.
                    </li>
                    <li>
                        <b>View Count.</b>
                        The number of views the clip has.
                    </li>
                    <li>
                        <b>Description.</b>
                        Various details about the clip including: the time when the clip was created, the username of the user who created the clip, the description of the clip.
                    </li>
                    <li>
                        <b>Like Button.</b>
                        For registered users, this is a button that allows you to like/un-like a clip. A green thumbs up button indicates that you have liked the clip.
                    </li>
                    <li>
                        <b>Clip Share Button.</b>
                        Displays a dialog box with a URL that you can copy and paste to share the clip.
                    </li>
                    <li>
                        <b>Comments.</b>
                        Registered users can see comments created by other users and add comments of their own.
                    </li>
                </ol>
            </div>

            <div class="mt-4">
                <h2 id="cliplibrary">Clip Library</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/vod/clip_library.jpg"
                    alt="The SquadOV clip library."
                    caption="Show me the clip level!"
                />

                The clip library displays a grid of all clips created by you and your squadmates.
                Note that the clips displayed will be different depending on how you got to the clip library screen.
                If you navigate to the clip library screen from the app navigation bar, you will see all clips from all matches.
                If you navigate to the clip library from the VOD picker for a single match, the clip library screen will only show clips that you and your squadmates have created for that match.
                Clicking on a clip will bring you to the <NuxtLink to="/user-manual/vod#clipviewer">Clip Viewer</NuxtLink> page.
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class Vod extends Vue {

}

</script>