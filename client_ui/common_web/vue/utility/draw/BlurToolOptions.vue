<template>
    <div>
        <v-slider
            :value="tool.blurAmount"
            @input="tool.blurAmount = arguments[0]"
            :min="1"
            :max="32"
            :step="1"
            label="Blur Size"
        >
            <template v-slot:append>
                <div class="font-weight-bold text-subtitle-2">
                    {{ tool.blurAmount }}
                </div>
            </template>
        </v-slider>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { BlurTool } from '@client/js/draw/tools/blur'

@Component
export default class BlurToolOptions extends Vue {
    @Prop({required: true})
    tool!: BlurTool
}

</script>