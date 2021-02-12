<template>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import * as pi from '@client/js/pages'

@Component
export default class PerformanceComponentChooser extends Vue {

    mounted() {
        this.$router.replace({
            name: pi.VizStatsPageId,
        })
    }
}

</script>

<style scoped>

</style>