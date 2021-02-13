<template>
    
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import * as pi from '@client/js/pages'

@Component
export default class WowDefaultGameLog extends Vue {
    mounted() {
        this.$router.replace({
            name: pi.WowEncounterLogPageId,
            params: this.$route.params,
            query: this.$route.query
        })

    }
}

</script>