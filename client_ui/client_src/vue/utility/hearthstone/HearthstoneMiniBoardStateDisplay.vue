<template>
    
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { HearthstoneMatchSnapshot } from '@client/js/hearthstone/hearthstone_snapshot'

@Component
export default class HearthstoneMiniBoardStateDisplay extends Vue {
    @Prop({required: true})
    snapshot!: HearthstoneMatchSnapshot | null
}

</script>