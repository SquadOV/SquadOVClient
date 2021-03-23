<template>
    <clip-library
        :user-id="userId"
        only-favorite
        title="Clip Favorites"
    >
    </clip-library>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import ClipLibrary from '@client/vue/utility/vods/ClipLibrary.vue'

@Component({
    components: {
        ClipLibrary
    }
})
export default class UserClipFavorites extends Vue {
    @Prop({required: true})
    userId!: number
}

</script>