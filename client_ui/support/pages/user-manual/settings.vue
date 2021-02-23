<template>
    <div>
        <h1>Welcome to SquadOV!</h1>
        <v-divider></v-divider>

        <div class="text-body-1">
            <div>
                <h2 id="video">Video Recording</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/settings/video_settings.jpg"
                    alt="SquadOV video capture settings."
                    caption="Change the quality of the output VOD's video."
                />

                You can find these settings in the <NuxtLink to="/user-manual/dashboard#recording">Recording Status</NuxtLink> section of the dashboard.

                <div class="mt-4">
                    There are currently two main controls for changing the quality of your VOD recordings: quality (resolution) and FPS (frames per second).
                </div>

                <div class="mt-4">
                    <i>Quality.</i>
                    This controls the maximum number of pixels SquadOV will use as the height of the VOD.
                    SquadOV will automatically choose a width that maintains your game’s aspect ratio.
                    If you select 1080p, SquadOV will use 1080 pixels, 720p, 720 pixels, 480p, 480 pixels, etc.
                    Thus, for example, if you select 720p and you record a full-screen game with a resolution of 2560x1440, your resulting VOD will have a resolution of 1280x720.
                </div>

                <div class="mt-4">
                    <i>FPS.</i>
                    This controls the target frames per second of the resulting VOD.
                    Due to the real-time performance nature of recording video games, while the resulting VOD may have the specified number of frames per second, the video may look like it has a lower amount of frames due to the fact that your computer was not able to process each frame in time.
                </div>
            </div>

            <div class="mt-4">
                <h2 id="audio">Audio Recording</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/settings/audio_settings.jpg"
                    alt="SquadOV audio capture settings."
                    caption="Change the quality of the output VOD's audio."
                />

                You can find these settings in the <NuxtLink to="/user-manual/dashboard#recording">Recording Status</NuxtLink> section of the dashboard.

                <div class="mt-4">
                    SquadOV will automatically record all desktop sounds (output) and microphone sounds (input).
                    By default, SquadOV will choose whatever you’ve set as your default playback/recording device in Windows.
                    If you wish to change which device you record from, select the appropriate device from the drop-down menu.
                    The slider underneath the device drop-down bar can be used to adjust the volume of that device in the final VOD.
                </div>

            </div>

            <div class="mt-4">
                <h2 id="system">System</h2>

                <GenericImage
                    class="my-4"
                    src="/assets/user-manual/settings/system_settings.jpg"
                    alt="SquadOV system settings."
                    caption="Change how SquadOV interacts with your OS."
                />

                These settings can be found in the <NuxtLink to="/user-manual/dashboard#appnav">App Navigation</NuxtLink> bar by clicking the Settings button under the User Profile section.

                <div class="mt-4">
                    These settings detail how SquadOV interacts with your operating system.
                    <ul>
                        <li>
                            <b>Run SquadOV on startup.</b>
                            If enabled, SquadOV will launch every time your login.
                        </li>
                        <li>
                            <b>Minimize SquadOV to system tray.</b>
                            If enabled, closing out of the SquadOV window will not exit the application. Instead, it will hide the window until you re-show it by clicking on the SquadOV icon in your system tray.
                        </li>
                    </ul>
                </div>
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

@Component
export default class Settings extends Vue {

}

</script>