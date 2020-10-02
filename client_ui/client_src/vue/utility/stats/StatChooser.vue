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
import { StatTree, StatTreeNode, buildVuetifyTreeViewItemFromStatTreeNode } from '@client/js/stats/stat_tree'

@Component
export default class StatChooser extends Vue {
    @Prop({type: Array, required: true})
    value!: string[]

    @Prop({type: Array, required: true})
    availableStats!: string[]

    get statTree() : StatTree {
        return new StatTree(this.availableStats)
    }

    get itemStats(): any[] {
        return this.statTree.rootNodes.map((ele : StatTreeNode) => buildVuetifyTreeViewItemFromStatTreeNode(ele)))
    }
}

</script>

<style scoped>

</style>