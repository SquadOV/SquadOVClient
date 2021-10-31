<template>
    <div id="appInner">
        <div class="d-flex flex-column flex-grow-1">
            <router-view v-if="hasValidSession"></router-view>
            <template v-else>
                <div
                    class="
                        d-flex
                        full-height full-width
                        justify-center
                        align-center
                        flex-column
                    "
                >
                    <v-progress-circular indeterminate size="64">
                    </v-progress-circular>

                    <div class="text-h6 my-4">
                        An error occurred while connecting to SquadOV...Trying
                        to reconnect.
                    </div>

                    <status-display style="width: 500px"></status-display>
                </div>
            </template>

            <div id="footer" class="flex-grow-0 py-1" v-if="showNav">
                <div class="text-center ma-0 text-overline footer-text">
                    SquadOV v{{ version }} - Created with
                    <i class="fas fa-heart"></i> by gamers for gamers.
                </div>
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Component from 'vue-class-component'
import { version } from '@client/package.json'
import App from '@client/vue/App.vue'

@Component({
    components: {
    }
})
export default class WebApp extends App {
    version: string = version
}

</script>