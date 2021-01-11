<template>
    <div id="appInner">
        <app-nav class="flex-grow-0" v-if="hasValidSession"></app-nav>            
        <div class="flex-grow-1">
            <router-view v-if="hasValidSession"></router-view>
            <template v-else>
                <div class="d-flex full-height full-width justify-center align-center flex-column">
                    <v-progress-circular indeterminate size="64">
                    </v-progress-circular>

                    <div class="text-h6 mt-4">
                        An error occurred while connecting to SquadOV...Trying to reconnect.
                    </div>
                </div>
            </template>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import AppNav from '@client/vue/AppNav.vue'
@Component({
    components: {
        AppNav,
    }
})
export default class WebApp extends Vue {
    get hasValidSession(): boolean {
        return this.$store.state.hasValidSession
    }
}

</script>

<style scoped>

#appInner {
    display: flex;
    flex-direction: column;
    height: 100%;
}

</style>