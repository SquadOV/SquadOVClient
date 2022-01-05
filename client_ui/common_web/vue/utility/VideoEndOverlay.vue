<template>
    <div class="endOverlay" @keydown.esc="closeOverlay" tabindex="0">
        <v-card id="background" height="100%" width="100%">
            <v-btn
                class="closeButton"
                color="gray"
                @click="closeOverlay"
                icon
                outlined
            >
                <v-icon>
                    mdi-close
                </v-icon>
            </v-btn>
            <div id="contentButtons">
                <v-row justify="center">
                    <v-btn
                        color="white"
                        class="buttons"
                        @click="rewatchVideo"
                        icon
                        x-large
                        min-height="144px"
                        plain
                    >
                        <v-icon size="144px">
                            mdi-replay
                        </v-icon>
                    </v-btn>
                </v-row>
                <v-row justify="center">
                    <match-share-button
                        v-if="!!matchUuid"
                        class="buttons"
                        :match-uuid="matchUuid"
                        :game="game"
                        :permissions="permissions"
                        :full-path="$route.fullPath"
                        :timestamp="timestamp"
                        :user-id="userId"
                        :is-local="isLocal"
                        :graphqlStats="graphqlStats"
                        :shareBtnText="false"
                    >
                    </match-share-button>
                    <match-share-button
                        v-else-if="!!clipUuid"
                        class="buttons"
                        :clip-uuid="clipUuid"
                        :permissions="permissions"
                        :full-path="$route.fullPath"
                        :timestamp="timestamp"
                        :shareBtnText="false"
                        no-clip
                    >
                    </match-share-button>
                </v-row>
            </div>
        </v-card>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Watch, Prop } from 'vue-property-decorator'
import MatchShareButton from '@client/vue/utility/squadov/MatchShareButton.vue'
import { MatchVideoSharePermissions } from '@client/js/squadov/share'
import { SquadOvGames } from '@client/js/squadov/game'
import { StatPermission } from '@client/js/stats/statPrimitives'

@Component({
    components: {
        MatchShareButton
    }
})
export default class VideoEndOverlay extends Vue {
    // Props for Match Sharing
    @Prop()
    matchUuid!: string | undefined

    @Prop()
    clipUuid!: string | undefined

    @Prop()
    game!: SquadOvGames

    @Prop({ required: true })
    permissions!: MatchVideoSharePermissions | null

    @Prop({ type: Boolean, default: false })
    noClip!: boolean

    @Prop()
    graphqlStats!: StatPermission[]

    @Prop({ required: true })
    fullPath!: string

    @Prop({ default: 0 })
    timestamp!: number

    @Prop()
    userId!: number

    @Prop({ type: Boolean, default: false })
    isLocal!: boolean

    // End of Props for Match Sharing

    showOverlay: boolean = true

    @Prop({ type: Number, default: 26 })
    zIndex!: number

    $refs!: {
        top: HTMLElement,
    }

    get clipOrGame(): string {
        if (!!this.matchUuid) {
            return this.matchUuid
        } else if (!!this.clipUuid) {
            return this.clipUuid
        }
        return ""
    }

    closeOverlay() {
        this.$emit('closeOverlay')
    }

    rewatchVideo() {
        this.$emit('replayVOD')
    }

    get overlayStyle(): any {
        return {
            'z-index': this.zIndex - 3
        }
    }

}

</script>

<style scoped>
.endOverlay {
    position: absolute;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
}

#background {
    z-index: 0;
    opacity: 0.8;
    backdrop-filter: blur(6px);
}

#contentButtons {
    position: absolute;
    top: 40%;
    width: 99%;
}

.closeButton {
    position: absolute;
    top: 3%;
    right: 3%;
}

.buttons {
    margin-bottom: 25px;
}
</style>