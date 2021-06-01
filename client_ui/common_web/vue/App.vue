<template>
    <keep-alive :max="3">
        <router-view :key="key"></router-view>
    </keep-alive>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { routeLevelToKey } from '@client/js/routes'

@Component
export default class App extends Vue {
    get key(): string {
        // We need to use a custom key here. We only want
        // to re-render this router view when the top level route
        // changes - not when a child level route changes.
        return routeLevelToKey(this.$route, 1)
    }
}

</script>