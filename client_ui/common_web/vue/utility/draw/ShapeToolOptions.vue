<template>
    <div>
        <v-btn-toggle
            :value="tool.activeShapeTool"
            @change="tool.activeShapeTool = arguments[0]"
            mandatory
        >
            <v-btn :value="EShapeTool.Rectangle">
                <v-icon>
                    mdi-rectangle
                </v-icon>
            </v-btn>

            <v-btn :value="EShapeTool.Circle">
                <v-icon>
                    mdi-checkbox-blank-circle
                </v-icon>
            </v-btn>

            <v-btn :value="EShapeTool.Ellipse">
                <v-icon>
                    mdi-ellipse
                </v-icon>
            </v-btn>

            <v-btn :value="EShapeTool.Triangle">
                <v-icon>
                    mdi-triangle
                </v-icon>
            </v-btn>
        </v-btn-toggle>

        <v-slider
            class="mt-1"
            :value="tool.borderWidth"
            @input="tool.borderWidth = arguments[0]"
            :min="1"
            :max="16"
            :step="1"
            label="Border Width"
        >
            <template v-slot:append>
                <div class="font-weight-bold text-subtitle-2">
                    {{ tool.borderWidth }}
                </div>
            </template>
        </v-slider>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { MultiShapeTool, EShapeTool } from '@client/js/draw/tools/shapes/shape'

@Component
export default class ShapeToolOptions extends Vue {
    EShapeTool: any = EShapeTool

    @Prop({required: true})
    tool!: MultiShapeTool
}

</script>