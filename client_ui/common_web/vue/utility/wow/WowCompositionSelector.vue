<template>
    <div class="d-flex align-center">
        <div>
            {{ label }}
        </div>

        <div class="d-flex align-center ml-2">
            <wow-spec-chooser
                v-for="(v, idx) in internalValues"
                v-model="internalValues[idx]"
                :key="idx"
            >
            </wow-spec-chooser>
        </div>
        
        <v-btn
            color="error"
            icon
            :disabled="internalValues.length === 0"
            @click="removeFromComp"
        >
            <v-icon> 
                mdi-minus
            </v-icon>
        </v-btn>

        <v-btn
            color="success"
            icon
            @click="addToComp"
        >
            <v-icon> 
                mdi-plus
            </v-icon>
        </v-btn>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import WowSpecChooser from '@client/vue/utility/wow/WowSpecChooser.vue'

@Component({
    components: {
        WowSpecChooser,
    }
})
export default class WowCompositionSelector extends Vue {
    @Prop()
    value!: string[]

    @Prop({default: 'Team'})
    label!: string

    internalValues: number[][] = []

    @Watch('internalValues', {deep: true})
    onValueChange() {
        let ret: string[] = []
        for (let arr of this.internalValues) {
            ret.push(JSON.stringify(arr))
        }
        this.$emit('input', ret)
    }

    addToComp() {
        this.internalValues.push([])
    }

    removeFromComp() {
        if (this.internalValues.length === 0) {
            return
        }

        this.internalValues.pop()
    }
}

</script>