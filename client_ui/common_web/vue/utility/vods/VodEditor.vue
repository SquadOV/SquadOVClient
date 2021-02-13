<template>
    <div>

    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { VodEditorContext } from '@client/js/vods/editor'
import VideoPlayer from '@client/vue/utility/VideoPlayer.vue'

@Component({
    components: {
        VideoPlayer
    }
})
export default class VodEditor extends Vue {
    @Prop({required: true})
    videoUuid!: string

    context: VodEditorContext | undefined = undefined

    @Watch('videoUuid')
    refreshContext() {
        this.context = new VodEditorContext(this.videoUuid)
    }
    
    mounted() {
        this.refreshContext()
    }

    beforeDestroy() {
        if (!!this.context) {
            this.context.close()
        }
        this.context = undefined
    }
}

</script>