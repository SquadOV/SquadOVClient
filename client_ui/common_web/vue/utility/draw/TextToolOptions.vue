<template>
    <div>
        <v-select
            :value="tool.fontFamily"
            @input="tool.fontFamily = arguments[0]"
            :items="familyFamilyOptions"
            dense
            hide-details
            outlined
        >
            <template v-slot:prepend>
                <div class="font-weight-bold text-subtitle-2">
                    Font
                </div>
            </template>

            <template v-slot:item="{item}">
                <div :style="`font-family: ${item};`">
                    {{ item }}
                </div>
            </template>
        </v-select>

        <v-slider
            :value="tool.fontSize"
            @input="tool.fontSize = arguments[0]"
            :min="2"
            :max="128"
            :step="2"
            label="Size"
            hide-details
        >
            <template v-slot:append>
                <div class="font-weight-bold text-subtitle-2">
                    {{ tool.fontSize }}
                </div>
            </template>
        </v-slider>

        <v-btn-toggle
            dense
            multiple
        >
            <v-btn
                :input-value="tool.bold"
                @click="tool.bold = !tool.bold"
            >
                <v-icon>mdi-format-bold</v-icon>
            </v-btn>

            <v-btn
                :input-value="tool.italic"
                @click="tool.italic = !tool.italic"
            >
                <v-icon>mdi-format-italic</v-icon>
            </v-btn>

            <v-btn
                :input-value="tool.underline"
                @click="tool.underline = !tool.underline"
            >
                <v-icon>mdi-format-underline</v-icon>
            </v-btn>
        </v-btn-toggle>

        <v-btn-toggle
            class="mt-2"
            :value="tool.align"
            @change="tool.align = arguments[0]"
            mandatory
        >
            <v-btn value="left">
                <v-icon>
                    mdi-format-align-left
                </v-icon>
            </v-btn>

            <v-btn value="center">
                <v-icon>
                    mdi-format-align-center
                </v-icon>
            </v-btn>

            <v-btn value="right">
                <v-icon>
                    mdi-format-align-right
                </v-icon>
            </v-btn>

            <v-btn value="justify">
                <v-icon>
                    mdi-format-align-justify
                </v-icon>
            </v-btn>
        </v-btn-toggle>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { TextTool } from '@client/js/draw/tools/text'

@Component
export default class TextToolOptions extends Vue {
    @Prop({required: true})
    tool!: TextTool

    get familyFamilyOptions(): string[] {
        return [
            'Open Sans',
            'Lobster Two',
            'Space Mono',
            'Comic Neue'
        ]
    }
}

</script>