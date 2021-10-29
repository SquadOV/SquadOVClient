<template>
    <loading-container
        :is-loading="recentMatches === null"
    >
        <template v-slot:default="{ loading }">
        </template>
    </loading-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'
import { Prop, Watch } from 'vue-property-decorator'
import { SquadOvGames, gameToPossibleWowTuple } from '@client/js/squadov/game'
import { createEmptyRecentMatchFilters, RecentMatch } from '@client/js/squadov/recentMatch'
import { apiClient, ApiData, HalResponse } from '@client/js/api'
import { DashboardPageId } from '@client/js/pages'

///#if DESKTOP
import { ipcRenderer } from 'electron'
///#endif

@Component({
    components: {
        LoadingContainer,
    }
})
export default class PostGameReport extends Vue {
    @Prop({required: true})
    game!: SquadOvGames

    @Prop({required: true})
    tm!: Date

    recentMatches: RecentMatch[] | null = null
    processingMatches: RecentMatch[] | null = null

    @Watch('game')
    @Watch('tm')
    refreshData() {
        // Get matches the current user played after the specified time for the game specified.
        this.recentMatches = null

        let filters = createEmptyRecentMatchFilters()
        filters.users = [ this.$store.state.currentUser.id ]

        let gameTuple = gameToPossibleWowTuple(this.game)
        filters.games = [ gameTuple[0] ]

        if (!!gameTuple[1]) {
            filters.wowReleases = [ gameTuple[1] ]
        }

        // TODO: Maybe even do a search for best moments to show instead?
        apiClient.listMyRecentMatches({
            next: null,
            start: 0,
            // 5 just seems like a reasonable number and we can filter them client side if need be
            end: 5,
            filters,
            profileId: undefined,
        }).then((resp : ApiData<HalResponse<RecentMatch[]>>) => {
            // Filter out VODs with MP4s so we can be sure that processing happened on that VOD.
            this.recentMatches = resp.data.data.filter((r: RecentMatch) => {
                return r.base.vod.videoTracks[0].segments[0].mimeType !== 'video/mp2t'
            })

            this.processingMatches = resp.data.data.filter((r: RecentMatch) => {
                return r.base.vod.videoTracks[0].segments[0].mimeType === 'video/mp2t'
            })

///#if DESKTOP
            // We need to actually tell the main desktop window that a post-game report was successfully loaded
            // so we can make sure the SquadOV window pops up.
            ipcRenderer.send('request-restore')
///#endif
        }).catch((err : any) => {
            console.error('Failed to get SquadOV matches for post-game report: ' + err);

            // In this case redirect back to the dashboard since something went terribly wrong here.
            this.$router.replace({
                name: DashboardPageId,
            })
        })
    }

    mounted() {
        this.refreshData()
    }
}

</script>