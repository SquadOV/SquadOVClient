<template>
    <generic-cohort-container
        title="Active Users"
        :cohort="cohort"
    >
    </generic-cohort-container>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import GenericCohortContainer from '@client/vue/cohorts/GenericCohortContainer.vue'
import { Cohorts } from '@client/ts/cohorts'

@Component({
    components: {
        GenericCohortContainer,
    }
})
export default class CohortActiveUsers extends Vue {
    get cohort(): Cohorts {
        return Cohorts.ActiveUsers
    }
}

</script>