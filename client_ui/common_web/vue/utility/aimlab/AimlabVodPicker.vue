<template>
    <generic-vod-picker
        :value="vod"
        :options="options"
        :match-uuid="taskId"
        :timestamp="timestamp"
        :game="game"
        :disable-favorite="disableFavorite"
        :enable-draw="enableDraw"
        @update:enableDraw="$emit('update:enableDraw', arguments[0])"
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
import { SquadOvGames } from '@client/js/squadov/game'

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

    @Prop()
    timestamp!: Date | undefined | null

    @Prop({type: Boolean, default: false})
    disableFavorite!: boolean

    @Prop({type: Boolean, default: false})
    enableDraw!: boolean

    get options(): VodAssociation[] {
        return [this.vod]
    }

    get game(): SquadOvGames {
        return SquadOvGames.AimLab
    }
}

</script>