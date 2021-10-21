<template>
    <div class="d-flex align-center">
        <div>
            {{ label }}:&nbsp;
        </div>

        <v-text-field
            v-model="lowerStr"
            @input="onStrValueChange"
            hide-details
            outlined
            single-line
            dense
            type="number"
        >
        </v-text-field>

        <div class="mx-2">
            -
        </div>

        <v-text-field
            v-model="upperStr"
            @input="onStrValueChange"
            hide-details
            outlined
            single-line
            dense
            type="number"
        >
        </v-text-field>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'

@Component
export default class NumberRangeFilterUi extends Vue {
    @Prop({default: 'Range'})
    label!: string

    @Prop()
    lower!: number | undefined
    lowerStr: string = ''

    @Prop()
    upper!: number | undefined
    upperStr: string = ''

    toValue(s: string): number | undefined {
        let t = s.trim()
        if (t === '') {
            return undefined
        } else {
            return parseInt(s)
        }
    }

    onStrValueChange() {
        this.$emit('update:lower', this.toValue(this.lowerStr))
        this.$emit('update:upper', this.toValue(this.upperStr))
    }
}

</script>