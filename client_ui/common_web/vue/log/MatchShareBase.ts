import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import { MatchVideoSharePermissions } from '@client/js/squadov/share'
import { apiClient, ApiData } from '@client/js/api'
import { SquadOvGames } from '@client/js/squadov/game'
import { VodAssociation } from '@client/js/squadov/vod'

@Component
export default class MatchShareBase extends Vue {
    matchPermissions: MatchVideoSharePermissions | null = null
    vod : VodAssociation | null = null
    timestamp: number = 0
    
    @Prop({default: 0})
    inTs!: number

    mounted() {
        if (!isNaN(this.inTs)) {
            this.timestamp = this.inTs
        }
    }

    @Watch('vod')
    onVodChange(newVod: VodAssociation | null, oldVod: VodAssociation | null) {
        if (!oldVod && !!newVod && !!this.$store.state.currentUser) {
            apiClient.markUserActive().catch((err: any) => {
                console.error('Failed to mark user active', err)
            })
        }
    }

    refreshMatchPermissions(matchUuid: string, game: SquadOvGames) {
        if (!this.$store.state.currentUser) {
            return
        }

        this.matchPermissions = null
        apiClient.getGenericSharePermissions(matchUuid, undefined, game).then((resp: ApiData<MatchVideoSharePermissions>) => {
            this.matchPermissions = resp.data
        }).catch((err: any) => {
            console.error('Failed to get match share permissions: ', err)
        })
    }
}