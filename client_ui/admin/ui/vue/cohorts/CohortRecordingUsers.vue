<template>
    <generic-cohort-container
        title="Recording Users"
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
export default class CohortRecordingUsers extends Vue {
    get cohort(): Cohorts {
        return Cohorts.RecordingUsers
    }
}

</script>