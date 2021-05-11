<template>
    <div>
        <v-tooltip
            offset-y bottom
        >
            <template v-slot:activator="{on, attrs}">
                <v-img
                    :src="src"
                    :width="width"
                    :height="height"
                    :max-width="width"
                    :max-height="height"
                    contain
                    v-on="on"
                    v-bind="attrs"
                >
                </v-img>
            </template>
            
            <span v-if="!!account">{{ account.name }}</span>
        </v-tooltip>
    </div>
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