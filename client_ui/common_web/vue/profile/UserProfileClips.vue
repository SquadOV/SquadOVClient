<template>
    
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'

@Component
export default class UserProfileClips extends Vue {
    @Prop({required: true})
    userId!: number

    @Watch('userId')
    refreshData() {

    }

    mounted() {
        this.refreshData()
    }
}

</script>