<template>
    <div>
        <v-slider
            :value="tool.width"
            @input="changeWidth"
            :min="1"
            :max="32"
            :step="1"
            label="Width"
        >
            <template v-slot:append>
                <div class="font-weight-bold text-subtitle-2">
                    {{ tool.width }}
                </div>
            </template>
        </v-slider>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { BrushTool } from '@client/js/draw/tools/brush'

@Component
export default class BrushToolOptions extends Vue {
    @Prop({required: true})
    tool!: BrushTool

    changeWidth(v: number) {
        this.tool.setWidth(v)
    }
}

</script>