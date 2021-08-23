<template>
    <div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import LoadingContainer from '@client/vue/utility/LoadingContainer.vue'

@Component({
    components: {
        LoadingContainer,
    }
})
export default class UserProfile extends Vue {
    @Prop({required: true})
    userId!: number
}

</script>