import Vue from 'vue'
import Component from 'vue-class-component'
import { MatchVideoSharePermissions } from '@client/js/squadov/share'
import { apiClient, ApiData } from '@client/js/api'
import { SquadOvGames } from '@client/js/squadov/game'

@Component
export default class MatchShareBase extends Vue {
    matchPermissions: MatchVideoSharePermissions | null = null

    refreshMatchPermissions(matchUuid: string, game: SquadOvGames) {
        this.matchPermissions = null
        apiClient.getMatchSharePermissions(matchUuid, game).then((resp: ApiData<MatchVideoSharePermissions>) => {
            this.matchPermissions = resp.data
        }).catch((err: any) => {
            console.log('Failed to get match share permissions: ', err)
        })
    }
}