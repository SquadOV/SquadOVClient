<template>
    
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'

@Component
export default class CommunityDiscover extends Vue {
    @Prop({default: ''})
    search!: string
}

</script>