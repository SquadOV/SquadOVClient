<template>
    <div id="appInner">
        <app-nav class="flex-grow-0"></app-nav>
        
        <div class="flex-grow-1">
            <router-view></router-view>
        </div>

        <footer class="flex-grow-0">
            <p class="text-center ma-0 text-overline">
                SquadOV v{{ version }} - Created with <i class="fas fa-heart"></i> by Michael "b3h47pte" Bao.
            </p>
        </footer>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import AppNav from '@client/vue/AppNav.vue'
import { version } from '@client/../package.json'

@Component({
    components: {
        AppNav
    }
})
export default class App extends Vue {
    version: string = version
}

</script>

<style scoped>

#appInner {
    display: flex;
    flex-direction: column;
    height: 100%;
}

</style>