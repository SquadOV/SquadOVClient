<template>
    
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { LiveProcessRecord } from '@client/js/system/settings'

@Component
export default class LiveProcessRecordDisplay extends Vue {
    @Prop({required: true})
    record!: LiveProcessRecord
}

</script>