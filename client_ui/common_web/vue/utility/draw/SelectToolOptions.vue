<template>
    <div class="text-subtitle-2 font-weight-bold">
        No options available.
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { SelectTool } from '@client/js/draw/tools/select'

@Component
export default class SelectToolOptions extends Vue {
    @Prop({required: true})
    tool!: SelectTool
}

</script>