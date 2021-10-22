<template>
    <div>
        <template v-if="showFilters">
            <v-btn
                class="pa-0"
                text
                @click="showFilters = false"
                x-small
            >
                <v-icon x-small>
                    mdi-chevron-down
                </v-icon>
                Hide {{ label }}
            </v-btn>
        </template>

        <template v-else>
            <v-btn
                class="pa-0"
                text
                @click="showFilters = true"
                x-small
            >
                <v-icon x-small>
                    mdi-chevron-right
                </v-icon>
                Show {{ label }}
            </v-btn>
        </template>

        <div v-if="showFilters">
            <slot></slot>
        </div>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'

@Component
export default class GenericMatchFilterUi extends Vue {
    showFilters: boolean = false
    
    @Prop({default: 'Filters'})
    label!: string
}

</script>