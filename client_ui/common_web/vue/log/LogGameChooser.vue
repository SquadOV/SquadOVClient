<template>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import * as pi from '@client/js/pages'

@Component
export default class LogGameChooser extends Vue {
    mounted() {
        // TODO: Maybe have a way to select the user's preferred game or something?
        this.$router.replace({
            name: pi.AimlabLogPageId,
            params: this.$route.params,
            query: this.$route.query
        })
    }
}

</script>

<style scoped>

.choice {
    margin-left: 8px;
    margin-right: 8px;
}

</style>