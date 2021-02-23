<template>
    <div>
        <v-list-group v-model="item.open" no-action v-if="!!item.children">
            <template v-slot:activator>
                <v-list-item class="pl-0">
                    <v-list-item-title>{{ item.text }} </v-list-item-title>
                </v-list-item>
            </template>

            <v-list class="pl-4" dense expand>
                <NavBarItem
                    v-for="(subitem, subidx) in item.children"
                    :key="subidx"
                    :item="subitem"
                />
            </v-list>
        </v-list-group>

        <v-list-item :to="item.to" v-else>
            <v-list-item-title>{{ item.text }} </v-list-item-title>
        </v-list-item>
    </div>
</template>

<script lang="ts">


import Vue from 'vue'
import Component from 'nuxt-class-component'
import { Prop } from 'nuxt-property-decorator'

@Component
export default class NavBarItem extends Vue {
    @Prop()
    item!: any
}


</script>