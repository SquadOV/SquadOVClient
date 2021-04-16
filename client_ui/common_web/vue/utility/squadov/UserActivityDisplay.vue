<template>
    <div class="d-flex align-center">
        <div v-if="currentStatus.activity === SquadOvActivity.Offline" class="dot offline">
        </div>

        <div v-else-if="currentStatus.activity === SquadOvActivity.Online" class="dot online">
        </div>

        <div v-else class="d-flex">
            <template v-for="(g, idx) in currentStatus.game">
                <v-img
                    :class="`${currentStatus.activity === SquadOvActivity.Recording ? 'recording' : ''}`"
                    :key="idx"
                    width="24px"
                    max-width="24px"
                    height="24px"
                    max-height="24px"
                    contain
                    :src="$root.generateAssetUri(gameToIcon(g))"
                >
                </v-img>
            </template>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { TrackedUserStatus, SquadOvActivity } from '@client/js/squadov/status'
import { gameToIcon } from '@client/js/squadov/game'

@Component
export default class UserActivityDisplay extends Vue {
    SquadOvActivity = SquadOvActivity
    gameToIcon = gameToIcon
    
    @Prop({required: true})
    status!: TrackedUserStatus | undefined

    get currentStatus(): TrackedUserStatus {
        if (!!this.status) {
            return this.status
        } else {
            return {
                activity: SquadOvActivity.Offline,
                game: [],
            }
        }
    }
}

</script>

<style scoped>

.dot {
    height: 24px;
    width: 24px;
    border: 2px solid white;
    border-radius: 50%;
    display: inline-block;
}

.dot.online {
    background-color: green;
}

.dot.offline {
    background-color: gray;
}

.recording {
    animation: 2s linear 0s infinite normal record-pulse;
}

@keyframes record-pulse {
    0% {
        opacity: 1.0;
    }

    50% {
        opacity: 0.0;
    }
}

</style>