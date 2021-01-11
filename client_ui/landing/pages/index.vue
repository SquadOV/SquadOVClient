<template>
    <div>
        <div id="headerDiv">
            <video ref="video" id="headerVideo" muted>
            </video>

            <div id="headerContent" class="d-flex flex-column align-center justify-center">
                <h1>SquadOV</h1>
                <h2>Free automatic VOD recording, match history, and stat analysis to help you improve.</h2>

                <div class="mt-4">
                    <v-btn class="download-button" color="success" :href="downloadLink">
                        <div class="d-flex">
                            <v-icon class="mr-4">
                                mdi-microsoft-windows
                            </v-icon>

                            <div class="d-flex flex-column justify-center">
                                <span class="download-text">Client (x64)</span>
                                <span class="download-text">{{version}} (91MB)</span>
                            </div>
                        </div>
                    </v-btn>

                    <v-btn class="download-button" color="secondary" href="https://app.squadov.gg">
                        <div class="d-flex">
                            <v-icon class="mr-4">
                                mdi-web
                            </v-icon>

                            <div class="d-flex flex-column justify-center">
                                <span class="download-text">Web Client</span>
                            </div>
                        </div>
                    </v-btn>
                </div>
            </div>
        </div>

        <v-row class="mt-4" no-gutters justify="center">
            <v-col cols="9">
                <div class="d-flex justify-center text-h4 font-weight-bold">
                    Supported Games
                </div>

                <div class="d-flex mt-4 game-div">
                    <NuxtLink to="/aimlab">
                        <div :class="`d-flex flex-column align-center mx-4 single-game-div ${$nuxt.$route.path == '/aimlab' ? 'selected-game-div' : ''}`">
                            <v-img
                                src="assets/aimlab_box.jpg"
                                height="250px"
                                width="188px"
                                contain
                                alt="Aim Lab"
                            >
                            </v-img>

                            <div class="text-overline beta">
                                Beta
                            </div>

                            <div class="mt-2 text-overline">
                                Aim Lab
                            </div>
                        </div>
                    </NuxtLink>

                    <NuxtLink to="/hearthstone">
                        <div :class="`d-flex flex-column align-center mx-4 single-game-div ${$nuxt.$route.path == '/hearthstone' ? 'selected-game-div' : ''}`">
                            <v-img
                                src="assets/hearthstone_box.jpg"
                                height="250px"
                                width="188px"
                                contain
                                alt="Hearthstone"
                            >
                            </v-img>

                            <div class="text-overline beta">
                                Beta
                            </div>

                            <div class="mt-2 text-overline">
                                Hearthstone
                            </div>
                        </div>
                    </NuxtLink>

                    <div :class="`d-flex flex-column align-center mx-4 single-game-div ${$nuxt.$route.path == '/lol' ? 'selected-game-div' : ''}`">
                        <v-img
                            src="assets/lol_box.jpg"
                            height="250px"
                            width="188px"
                            contain
                            alt="League of Legends"
                        >
                        </v-img>

                        <div class="text-overline dev">
                            Coming Soon
                        </div>

                        <div class="mt-2 text-overline">
                            League of Legends
                        </div>
                    </div>

                    <div :class="`d-flex flex-column align-center mx-4 single-game-div ${$nuxt.$route.path == '/tft' ? 'selected-game-div' : ''}`">
                        <v-img
                            src="assets/tft_box.jpg"
                            height="250px"
                            width="188px"
                            contain
                            alt="Teamfight Tactics"
                        >
                        </v-img>

                        <div class="text-overline dev">
                            Coming Soon
                        </div>

                        <div class="mt-2 text-overline">
                            Teamfight Tactics
                        </div>
                    </div>

                    <NuxtLink to="/valorant">
                        <div :class="`d-flex flex-column align-center mx-4 single-game-div ${$nuxt.$route.path == '/valorant' ? 'selected-game-div' : ''}`">
                            <v-img
                                src="assets/valorant_box.jpg"
                                height="250px"
                                width="188px"
                                contain
                                alt="Valorant"
                            >
                            </v-img>

                            <div class="text-overline alpha">
                                Alpha
                            </div>

                            <div class="mt-2 text-overline">
                                Valorant
                            </div>
                        </div>
                    </NuxtLink>

                    <NuxtLink to="/wow">
                        <div :class="`d-flex flex-column align-center mx-4 single-game-div ${$nuxt.$route.path == '/wow' ? 'selected-game-div' : ''}`">
                            <v-img
                                src="assets/wow_box.jpg"
                                height="250px"
                                width="188px"
                                contain
                                alt="World of Warcraft"
                            >
                            </v-img>

                            <div class="text-overline alpha">
                                Alpha
                            </div>

                            <div class="mt-2 text-overline">
                                World of Warcraft
                            </div>
                        </div>
                    </NuxtLink>
                </div>
            </v-col>
        </v-row>

        <NuxtChild></NuxtChild>

        <v-row class="mt-4" no-gutters justify="center">
            <v-col cols="9">
                <div id="final-cta" class="d-flex flex-column align-center justify-center">
                    <div class="text-h5">
                        Download SquadOV today to start automatically recording and analyzing your VODs.
                    </div>

                    <div class="mt-4">
                        <v-btn class="download-button" color="success" :href="downloadLink">
                            <div class="d-flex">
                                <v-icon class="mr-4">
                                    mdi-microsoft-windows
                                </v-icon>

                                <div class="d-flex flex-column">
                                    <span class="download-text">Client (x64)</span>
                                    <span class="download-text">{{version}} (91MB)</span>
                                </div>
                            </div>
                        </v-btn>

                        <v-btn class="download-button" color="secondary" href="https://app.squadov.gg">
                            <div class="d-flex">
                                <v-icon class="mr-4">
                                    mdi-web
                                </v-icon>

                                <div class="d-flex flex-column justify-center">
                                    <span class="download-text">Web Client</span>
                                </div>
                            </div>
                        </v-btn>
                    </div>
                </div>
            </v-col>
        </v-row>

        <footer class="ma-4">
            <p class="d-flex flex-column align-center text-center ma-0">
                <span class="footer-text">Created with <v-icon color="pink">mdi-heart</v-icon> for gamers by gamers.</span>
                <span class="legal-text">SquadOV isn't endorsed by Riot Games and doesn't reflect the views or opinions of Riot Games or anyone officially involved in producing or managing Riot Games properties. Riot Games, and all associated properties are trademarks or registered trademarks of Riot Games, Inc.</span>
                <span class="legal-text">SquadOV is not affiliated with Blizzard Entertainment.</span>
            </p>
        </footer>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'nuxt-class-component'

const availableHeaderVideos : string[] = [
    'assets/videos/aimlab.mp4',
    'assets/videos/valorant_clip.mp4',
    'assets/videos/hearthstone_clip.mp4'
]

@Component
export default class Index extends Vue {
    $refs!: {
        video: HTMLVideoElement
    }

    head() : any {
        return {
            title: 'SquadOV - Automatic VOD Analysis for your Squad',
            htmlAttrs: {
                lang: 'en'
            },
            meta: [
                {
                    charset: 'utf-8'
                },
                {
                    name: 'viewport',
                    content: 'width=device-width, initial-scale=1'
                },
                {
                    hid: 'description',
                    name: 'description',
                    content: 'SquadOV is your tool for gaming VOD analysis. We keep track of your games and tasks so that you can visualize your performance over time and relive your best gaming moments.'
                }
            ]
        }
    }

    get version(): string {
        return 'v0.4.10'
    }

    get downloadLink() : string {
        return 'https://us-central1.content.squadov.gg/builds/SquadOV.exe'
    }

    currentVideo: number = 0
    switchVideo() {
        let nextVideo: number = (this.currentVideo + 1) % availableHeaderVideos.length
        this.fadeInVideo()
        this.$refs.video.setAttribute('src', availableHeaderVideos[nextVideo])
        this.$refs.video.play()
        this.currentVideo = nextVideo
    }

    mounted() {
        this.switchVideo()
        this.$refs.video.addEventListener('ended', () => {
            this.switchVideo()
        })
        this.$refs.video.addEventListener('timeupdate', (e : any) => {
            let remaining = this.$refs.video.duration - this.$refs.video.currentTime 
            if (remaining < 1.5) {
                this.fadeOutVideo()
            }
        })
    }

    fadeInVideo() {
        this.$refs.video.style.opacity = '1.0'
    }

    fadeOutVideo() {
        this.$refs.video.style.opacity = '0.0'
    }
}

</script>

<style scoped>

#headerDiv {
    max-height: 600px;
    height: 600px;
    position: relative;
}

#headerVideo {
    height: 600px;
    width: 100%;
    object-fit: cover;
    object-position: 50% 50%;
    filter: blur(5px) opacity(50%);
    transition: opacity 1s ease-out;
}

#headerContent {
    position: absolute;
    z-index: 1;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
}

.download-button {
    height: 50px !important;
}

.download-text {
    text-align: left;
}

.game-div {
    flex-wrap: wrap;
    justify-content: center;
}

.game-div a {
    color: inherit;
    text-decoration: none;
}

.single-game-div {
    border: 2px solid transparent;
    margin: auto;
}

.selected-game-div {
    border: 2px solid yellow;
    margin: auto;
}

footer {
    width: 100%;
}

.footer-text {
    font-size: 15px;
}

.legal-text {
    font-size: 10px;
    width: 40%;
}

.carousel-image >>> .v-image__image--cover {
    background-position: top !important;
}

.dev {
    background-color: gray;
    width: 100%;
    line-height: 1rem;
    color: black;
    text-align: center;
}

.alpha {
    background-color: yellow;
    width: 100%;
    line-height: 1rem;
    color: black;
    text-align: center;
}

.beta {
    background-color: orange;
    width: 100%;
    line-height: 1rem;
    color: black;
    text-align: center;
}

#final-cta {
    max-height: 300px;
    height: 300px;
}

</style>