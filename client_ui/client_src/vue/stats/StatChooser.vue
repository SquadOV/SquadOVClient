<template>
    <v-treeview
        :value="value"
        @input="$emit('input', arguments[0])"
        selectable
        :items="itemStats"
        open-on-click
    >
    </v-treeview>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { statLibrary } from '@client/js/stats/statLibrary'

@Component
export default class StatChooser extends Vue {
    @Prop({type: Array, required: true})
    value!: string[]

    @Prop({type: Array, required: true})
    availableStats!: string[]

    get itemStats(): any[] {
        return statLibrary.vuetifyTreeViewItems
    }
}

</script>

<style scoped>

</style>