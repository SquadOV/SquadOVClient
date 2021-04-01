<template>
    <div v-if="talentId != 0 && (!hideIfSocket || !isSocket)">
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

            {{ name }}
        </v-tooltip>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop, Watch } from 'vue-property-decorator'
import { staticClient } from '@client/js/staticData'
import { wowCache, WowTalentStatic } from '@client/js/wow/staticCache'

@Component
export default class WowTalentIcon extends Vue {
    @Prop({required: true})
    talentId!: number

    @Prop({type: Number, default: 32})
    widthHeight!: number

    @Prop({type: Boolean, default: false})
    hideIfSocket!: boolean

    @Prop({required: true})
    patch!: string

    talent: WowTalentStatic | null = null

    get src(): string {
        return staticClient.getWowTalentIconUrl(this.patch, this.talentId)
    }

    get name(): string {
        if (!this.talent) {
            return ''
        }
        return this.talent.name
    }

    get isSocket(): boolean {
        if (!this.talent) {
            return false
        }
        return this.talent.socket > 0
    }

    @Watch('talentId')
    refreshTalent() {
        wowCache.getCache(this.patch).getTalent(this.talentId).then((resp: WowTalentStatic) => {
            this.talent = resp
        }).catch((err: any) => {
            console.log('Failed to get talent: ', err)
        })
    }

    mounted() {
        this.refreshTalent()
    }
}

</script>