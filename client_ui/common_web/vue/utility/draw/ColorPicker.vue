<template>
    <div class="picker-parent-div">
        <v-menu
            v-model="showSelector"
            offset-x
            :close-on-content-click="false"
        >
            <template v-slot:activator="{on}">
                <div class="flex-grow-1 mx-2" :style="previewStyle" @click="showSelector = true">
                </div>
            </template>

            <v-color-picker
                dot-size="12"
                hide-mode-switch
                mode="rgba"
                :value="value"
                @input="$emit('input', arguments[0])"
            >
            </v-color-picker>
        </v-menu>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { ColorA, colorAToCssString } from '@client/js/color'

@Component
export default class ColorPicker extends Vue {
    @Prop({default: 24})
    height!: number

    @Prop({required: true})
    value!: ColorA

    showSelector: boolean = false

    get previewStyle(): any {
        return {

            height: `${this.height}px`,
            'background-color': colorAToCssString(this.value),
            'border': '1px solid white',
        }
    }
}

</script>

<style scoped>

.picker-parent-div {
    flex-basis: 100%;
}

</style>