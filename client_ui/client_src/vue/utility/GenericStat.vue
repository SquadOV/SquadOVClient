<template>
    <v-row justify="center">
        <v-col cols="6">
            <v-tooltip bottom>
                <template v-slot:activator="{ on, attrs }">
                    <span v-bind="attrs" v-on="on">{{ name }}</span>
                </template>
                <span>{{name}}: {{ desc }}</span>
            </v-tooltip>
        </v-col>

        <v-col class="value-text" cols="6">
            {{ valueDisplay}}
        </v-col>
    </v-row>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'

@Component
export default class GenericStat extends Vue {
    @Prop()
    name! : string

    @Prop({default: ''})
    desc! : string

    @Prop({type: Number})
    value!: number

    @Prop({type: Boolean, default: false})
    percentage! : boolean

    get valueDisplay() : string {
        const v = 
            (this.percentage) ?
                this.value * 100.0 : this.value
        const txt = v.toFixed(2)
        return (this.percentage) ?
            `${txt}%` : txt
    }
}

</script>

<style scoped>

.value-text {
    font-weight: 700;
    color: white;
}

</style>