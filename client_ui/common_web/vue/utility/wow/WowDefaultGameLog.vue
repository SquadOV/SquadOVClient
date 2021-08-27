<template>
    
</template>

<script lang="ts">

import Component, { mixins } from 'vue-class-component'
import * as pi from '@client/js/pages'
import CommonComponent from '@client/vue/CommonComponent'

@Component
export default class WowDefaultGameLog extends mixins(CommonComponent) {
    mounted() {
        this.$router.replace({
            name: pi.WowEncounterLogPageId,
            params: this.$route.params,
            query: this.cleanQuery
        })

    }
}

</script>