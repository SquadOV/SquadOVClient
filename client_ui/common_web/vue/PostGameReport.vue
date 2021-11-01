<template>
    <div class="pa-4">
        <div class="text-h5 font-weight-bold">
            {{username}}, check out the VODs SquadOV recorded while you played {{ gameName }}!
        </div>
        <v-divider class="my-2"></v-divider>

        <loading-container
            :is-loading="recentMatches === null"
        >
            <template v-slot:default="{ loading }">
                <div class="anim-parent" v-if="!!recentMatches && !!processingMatches && !loading">
                    <template v-if="recentMatches.length > 0">
                        <recent-match-display
                            class="anim"
                            v-for="(rMatch, idx) in recentMatches"
                            :key="`recent-${idx}`"
                            :match="rMatch"
                            :style="`animation-delay: ${idx * 0.2}s`"
                            :use-local-vod-preview="rMatch.base.isLocal"
                        >

                        </recent-match-display>
                    </template>

                    <template v-if="processingMatches.length > 0">
                        <div class="text-h6 font-weight-bold anim mt-4" :style="`animation-delay: ${recentMatches.length * 0.2}s`">
                            Some of your videos are still processing...check back soon!
                        </div>
                        <v-divider class="my-2"></v-divider>

                        <recent-match-display
                            class="anim"
                            v-for="(pMatch, idx) in processingMatches"
                            :key="`processing-${idx}`"
                            :match="pMatch"
                            :style="`animation-delay: ${(idx + recentMatches.length + 1) * 0.2}s`"
                        >

                        </recent-match-display>
                    </template>
                </div>
            </template>
        </loading-container>
    </div>
</template>

<script lang="ts">

import Component, {mixins} from 'vue-class-component'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { Prop, Watch } from 'vue-property-decorator'
import { SquadOvGames, gameToPossibleWowTuple, gameToName } from '@client/js/squadov/game'
import { createEmptyRecentMatchFilters, RecentMatch } from '@client/js/squadov/recentMatch'
import { apiClient, ApiData, HalResponse } from '@client/js/api'
import RecentMatchDisplay from '@client/vue/utility/RecentMatchDisplay.vue'
import CommonComponent from '@client/vue/CommonComponent'

///#if DESKTOP
import { ipcRenderer } from 'electron'
///#endif

@Component({
    components: {
        LoadingContainer,
        RecentMatchDisplay,
    }
})
export default class PostGameReport extends mixins(CommonComponent) {
    @Prop({required: true})
    game!: SquadOvGames

    @Prop({required: true})
    tm!: Date

    recentMatches: RecentMatch[] | null = null
    processingMatches: RecentMatch[] | null = null

    get username(): string {
        if (!!this.$store.state.currentUser) {
            return this.$store.state.currentUser.username
        } else {
            return 'Wow'
        }
    }

    get gameName(): string {
        return gameToName(this.game)
    }

    @Watch('isActive')
    @Watch('game')
    @Watch('tm')
    refreshData() {
        if (!this.isActive) {
            return
        }

        // Get matches the current user played after the specified time for the game specified.
        this.recentMatches = null

        let filters = createEmptyRecentMatchFilters()
        filters.users = [ this.$store.state.currentUser.id ]

        let gameTuple = gameToPossibleWowTuple(this.game)
        filters.games = [ gameTuple[0] ]

        if (!!gameTuple[1]) {
            filters.wowReleases = [ gameTuple[1] ]
        }

        filters.timeStart = this.tm.getTime()

        // TODO: Maybe even do a search for best moments to show instead?
        apiClient.listMyRecentMatches({
            next: null,
            start: 0,
            end: 10,
            filters,
            profileId: undefined,
        }).then((resp : ApiData<HalResponse<RecentMatch[]>>) => {
            resp.data.data.sort((a: RecentMatch, b: RecentMatch) => {
                return b.base.tm.getTime() - a.base.tm.getTime()
            })

            console.log('get recent matches: ', resp.data.data)

            // Filter out VODs with MP4s so we can be sure that processing happened on that VOD.
            this.recentMatches = resp.data.data.filter((r: RecentMatch) => {
                return r.base.isLocal || r.base.vod.videoTracks[0].segments[0].mimeType !== 'video/mp2t'
            })

            this.processingMatches = resp.data.data.filter((r: RecentMatch) => {
                return !r.base.isLocal && r.base.vod.videoTracks[0].segments[0].mimeType === 'video/mp2t'
            })

///#if DESKTOP
            // We need to actually tell the main desktop window that a post-game report was successfully loaded
            // so we can make sure the SquadOV window pops up.
            if (this.recentMatches.length > 0 || this.processingMatches.length > 0) {
                ipcRenderer.send('request-restore')
            } else {
                this.$router.back()
            }
///#endif
        }).catch((err : any) => {
            console.error('Failed to get SquadOV matches for post-game report: ' + err);

            // In this case redirect back to wherever the user was earlier because something went wrong.
            this.$router.back()
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>

<style scoped>

.anim {
    opacity: 0.0;
    transform: translateX(100%);
    animation: fadeIn 2s forwards, slideLeft 1s forwards;
}

.anim-parent {
    overflow-x: hidden;
}

</style>