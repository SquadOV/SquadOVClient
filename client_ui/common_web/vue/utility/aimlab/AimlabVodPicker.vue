<template>
    <generic-vod-picker
        :value="vod"
        :options="options"
        :match-uuid="taskId"
    >
        <template v-slot:vod="{ivod, selected}">
            <v-img
                :src="$root.generateAssetUri(content.getTaskIcon(taskName))"
                max-width="48"
                max-height="48"
                contain
            >
            </v-img>
        </template>
    </generic-vod-picker>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { VodAssociation } from '@client/js/squadov/vod'
import { AimlabContent, getAimlabContent } from '@client/js/aimlab/aimlab_content'
import GenericVodPicker from '@client/vue/utility/vods/GenericVodPicker.vue'

@Component({
    components: {
        GenericVodPicker
    }
})
export default class AimlabVodPicker extends Vue {
    content : AimlabContent = getAimlabContent()

    @Prop({required: true})
    taskId! : string

    @Prop({required: true})
    taskName! : string

    @Prop({required: true})
    vod!: VodAssociation

    get options(): VodAssociation[] {
        return [this.vod]
    }
}

</script>