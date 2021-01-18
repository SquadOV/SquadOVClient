<template>
    <div>
        <v-tooltip bottom>
            <template v-slot:activator="{on, attrs}">
                <div
                    v-on="on"
                    v-bind="attrs"
                >
                    <v-img
                        :src="$root.generateAssetUri(src)"
                        :height="height"
                        :max-height="height"
                        contain
                    >
                    </v-img>
                </div>
            </template>

            <div class="font-caption font-weight-bold">
                {{ name }}
            </div>
        </v-tooltip>
    </div>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { LolStaticTargets } from '@client/js/lol/timeline'
@Component
export default class LolStaticTargetDisplay extends Vue {
    @Prop({required: true})
    target!: LolStaticTargets

    @Prop({default: 64})
    height!: number

    get name(): string {
        switch (this.target) {
            case LolStaticTargets.EDragon:
                return 'Dragon'
            case LolStaticTargets.EDragonAir:
                return 'Air Dragon'
            case LolStaticTargets.EDragonFire:
                return 'Fire Dragon'
            case LolStaticTargets.EDragonEarth:
                return 'Earth Dragon'
            case LolStaticTargets.EDragonWater:
                return 'Water Dragon'
            case LolStaticTargets.EDragonElder:
                return 'Elder Dragon'
            case LolStaticTargets.EBaron:
                return 'Baron Nashor'
            case LolStaticTargets.ERiftHerald:
                return 'Rift Herald'
            case LolStaticTargets.ETower:
                return 'Tower'
            case LolStaticTargets.EInhibitor:
                return 'Inhibitor'
            case LolStaticTargets.ESightWard:
                return 'Sight Ward'
            case LolStaticTargets.EControlWard:
                return 'Control Ward'
            case LolStaticTargets.EYellowTrinket:
                return 'Yellow Trinket'
            case LolStaticTargets.EBlueTrinket:
                return 'Blue Trinket'
            case LolStaticTargets.EWard:
                return 'Ward'
        }
        return ''
    }

    get src(): string {
        switch (this.target) {
            case LolStaticTargets.EDragon:
                return 'assets/lol/dragon_clipped.png'
            case LolStaticTargets.EDragonAir:
                return 'assets/lol/dragons/air.png'
            case LolStaticTargets.EDragonFire:
                return 'assets/lol/dragons/fire.png'
            case LolStaticTargets.EDragonEarth:
                return 'assets/lol/dragons/earth.png'
            case LolStaticTargets.EDragonWater:
                return 'assets/lol/dragons/water.png'
            case LolStaticTargets.EDragonElder:
                return 'assets/lol/dragons/elder.png'
            case LolStaticTargets.EBaron:
                return 'assets/lol/baron_clipped.png'
            case LolStaticTargets.ERiftHerald:
                return 'assets/lol/rift_herald_clipped.png'
            case LolStaticTargets.ETower:
                return 'assets/lol/turret_clipped.png'
            case LolStaticTargets.EInhibitor:
                return 'assets/lol/inhibitor_clipped.png'
            case LolStaticTargets.ESightWard:
            case LolStaticTargets.EControlWard:
            case LolStaticTargets.EYellowTrinket:
            case LolStaticTargets.EBlueTrinket:
            case LolStaticTargets.EWard:
                return 'assets/lol/ward_clipped.png'
        }
        return ''
    }
}

</script>