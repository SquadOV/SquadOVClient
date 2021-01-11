<template>
    <div id="appInner">
        <template v-if="userLoaded">    
            <app-nav class="flex-grow-0"></app-nav>            
            <div class="flex-grow-1">
                <router-view></router-view>
            </div>
        </template>
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
    get userLoaded(): boolean {
        return !!this.$store.state.currentUser
    }
}

</script>
