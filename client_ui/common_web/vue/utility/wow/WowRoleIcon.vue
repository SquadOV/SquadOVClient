<template>
    <v-img
        :src="$root.generateAssetUri(imagePath)"
        :max-width="widthHeight"
        :max-height="widthHeight"
        contain
    >
    </v-img>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { WowRole } from '@client/js/wow/character'

@Component
export default class WowRoleIcon extends Vue {
    @Prop({ required: true })
    value!: WowRole

    @Prop({type: Number, default: 32})
    widthHeight!: number

    get imagePath(): string {
        switch (this.value) {
            case WowRole.Tank:
                return 'assets/wow/roles/tank.png'
            case WowRole.Healer:
                return 'assets/wow/roles/healer.png'
            case WowRole.Dps:
                return 'assets/wow/roles/dps.png'
            default:
                return 'assets/wow/roles/unknown.png'
        }
    }
}

</script>