<template>
    <div class="d-flex">
        <!-- Instance Selector List -->
        <div>
        </div>

        <!-- Controls to move between the two lists -->
        <div>
        </div>

        <!-- Disabled Instance List -->
        <div>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { WowDisabledInstance } from '@client/js/system/settings'

@Component
export default class WowDisabledInstancesDisplay extends Vue {
    @Prop({required: true})
    value!: WowDisabledInstance[]
}

</script>