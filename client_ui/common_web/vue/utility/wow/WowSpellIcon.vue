<template>
    <div v-if="spellId != 0">
        <v-tooltip bottom>
            <template v-slot:activator="{on, attrs}">
                <v-img
                    :width="widthHeight"
                    :max-width="widthHeight"
                    :height="widthHeight"
                    :max-height="widthHeight"
                    contain
                    v-on="on"
                    v-bind="attrs"
                    :src="src"
                >
                </v-img>
            </template>

            {{ spellName }}
        </v-tooltip>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { staticClient } from '@client/js/staticData'
import { wowCache } from '@client/js/wow/staticCache'

@Component
export default class WowSpellIcon extends Vue {
    @Prop({required: true})
    spellId!: number

    @Prop({type: Number, default: 32})
    widthHeight!: number

    spellName: string = ''

    get src(): string {
        return staticClient.getWowSpellIconUrl(this.spellId)
    }

    @Watch('spellId')
    refreshName() {
        wowCache.bulkGetSpellNames([this.spellId]).then((resp: Map<number, string>) => {
            this.spellName = resp.get(this.spellId)!
        }).catch((err: any) => {
            console.log('Failed to get spell name: ', err)
        })
    }

    mounted() {
        this.refreshName()
    }
}

</script>