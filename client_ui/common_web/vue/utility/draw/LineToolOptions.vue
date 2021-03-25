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
import { LineTool } from '@client/js/draw/tools/line'

@Component
export default class LineToolOptions extends Vue {
    @Prop({required: true})
    tool!: LineTool

    changeWidth(v: number) {
        this.tool.setWidth(v)
    }
}

</script>