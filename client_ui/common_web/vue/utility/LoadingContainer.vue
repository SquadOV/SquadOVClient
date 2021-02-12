<template>
    <div class="full-parent-height full-width">
        <slot v-if="!isLoading" v-bind:loading="isLoading"></slot>
        <div class="d-flex justify-center align-center full-width" v-else>
            <v-progress-circular indeterminate size="64"></v-progress-circular>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'

@Component
export default class LoadingContainer extends Vue {
    @Prop({type: Boolean, required: true})
    isLoading! : boolean
}

</script>

<style scoped>

</style>