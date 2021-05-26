<template>
    <keep-alive :max="3">
        <router-view :key="$route.fullPath"></router-view>
    </keep-alive>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'

@Component
export default class App extends Vue {
}

</script>