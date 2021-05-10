<template>
    <v-img
        :src="src"
        :width="width"
        :height="height"
        :max-width="width"
        :max-height="height"
    >
    </v-img>
</template>

<script lang="ts">

import Vue from 'vue'
import Component from 'vue-class-component'
import { Prop } from 'vue-property-decorator'
import { SteamAccount } from '@client/js/steam/account'

@Component
export default class SteamAccountDisplay extends Vue {
    @Prop({type: Number, default: 64})
    width!: number

    @Prop({type: Number, default: 64})
    height!: number

    @Prop({required: true})
    account!: SteamAccount | undefined

    get src(): string {
        if (!!this.account && !!this.account.profileImageUrl) {
            return this.account.profileImageUrl
        } else {
            //@ts-ignore
            return this.$root.generateAssetUri('assets/steam/steam_default.png')
        }
    }
}

</script>