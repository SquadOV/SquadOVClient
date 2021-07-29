<template>
    <div class="d-flex align-center justify-center">
        <v-btn
            class="mr-4"
            color="warning"
            @click="$emit('input', value - 1)"
            v-if="value > 1"
            large
        >
            Previous
        </v-btn>

        <v-btn
            color="primary"
            @click="$emit('input', value + 1)"
            v-if="value != totalSteps"
            :disabled="!canGoNext"
            large
        >
            Next
        </v-btn>

        <v-btn
            color="success"
            :to="dashboardTo"
            :disabled="!canGoNext"
            large
            v-else
        >
            Finish
        </v-btn>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import * as pi from '@client/js/pages'

@Component
export default class SetupWizardStepper extends Vue {
    @Prop({required: true})
    value!: number

    @Prop({required: true})
    totalSteps!: number

    @Prop({type: Boolean, required: true})
    canGoNext!: boolean

    get dashboardTo(): any {
        return {
            name: pi.DashboardPageId
        }
    }
}

</script>