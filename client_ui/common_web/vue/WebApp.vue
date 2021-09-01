<template>
    <div id="appInner" :style="appInnerStyle">
        <app-nav class="flex-grow-0" v-if="hasValidSession && showNav"></app-nav>            
        <div class="d-flex flex-column flex-grow-1">
            <router-view v-if="hasValidSession"></router-view>
            <template v-else>
                <div class="d-flex full-height full-width justify-center align-center flex-column">
                    <v-progress-circular indeterminate size="64">
                    </v-progress-circular>

                    <div class="text-h6 my-4">
                        An error occurred while connecting to SquadOV...Trying to reconnect.
                    </div>

                    <status-display style="width: 500px;"></status-display>
                </div>
            </template>

            <div id="footer" class="flex-grow-0 py-1" v-if="showNav">
                <div class="text-center ma-0 text-overline footer-text">
                    SquadOV v{{ version }} - Created with <i class="fas fa-heart"></i> by gamers for gamers.
                </div>
            </div>
        </div>
    </div>
</template>

<script lang="ts">

import Component, {mixins} from 'vue-class-component'
import AppNav from '@client/vue/AppNav.vue'
import { version } from '@client/package.json'
import TopLevelComponent from '@client/vue/TopLevelComponent'

@Component({
    components: {
        AppNav,
    }
})
export default class WebApp extends mixins(TopLevelComponent) {
    version: string = version
    
    get hasValidSession(): boolean {
        return this.$store.state.hasValidSession
    }
}

</script>